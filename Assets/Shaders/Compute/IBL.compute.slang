import PuduCoreModule;
import PuduRandomModule;
import "Lib/PuduLightingModule.slang";


struct Input
{
    RWTexture2DArray<float4> output;
    SamplerCube input;
    float roughness;
    uint sampleCount;
    uint inputResolution;
    uint outputResolution;
}

struct IBLDiffuseData
{
    SamplerCube input;
    RWTexture2DArray<float4> output;
    uint inputResolution;
    uint outputResolution;
    uint sampleCount;
}

ParameterBlock<Input> material;
ParameterBlock<IBLDiffuseData> materialDiffuse;


// mipmap filtered samples (GPU Gems 3, 20.4)
float ComputeLod(float PDF, uint Width, uint Height, uint SampleCount) {
  float w = float(Width);
  float h = float(Height);
  float sampleCountF = float(SampleCount);
  return 0.5 * log2( 6.0 * w * h / (sampleCountF * PDF));
}



float3 DiffuseFilteredIBL_ImportanceSampled(SamplerCube EnvMap, float3 N, uint SampleCount)
{
        uint width, height;
        EnvMap.GetDimensions(width,height);

        float3 c = 0.;

        for(uint i = 0; i<SampleCount;i++)
        {
                float4 importanceSample = GetLambertianImportanceSample(i, N, SampleCount);
                float3 H = importanceSample.xyz;
                float pdf = importanceSample.w;

                float lod = ComputeLod(pdf,width,height, SampleCount);

                float3 sampleColor = EnvMap.SampleLevel(H,lod).xyz;

                c += sampleColor;
        }

        c /=SampleCount;
        return c;
}

float3 DiffuseFilteredIBL_BruteForce(SamplerCube EnvMap, float3 N, uint EnvMapResolution)
{
    float3 irradiance = 0.;
    float sampleCount = 0.;
    float invRes = 1./EnvMapResolution;

    for(uint face = 0; face<6;++face)
    {
        for(uint x= 0; x<EnvMapResolution;++x)
        {
            for(uint y = 0; y<EnvMapResolution;++y)
            {
                float2 uv = (float2(x,y) + 0.5) * invRes;
                float2 pos = 2.f * uv - 1.f;

                float3 sampleDir = CubeCoordToDir(uv.x,uv.y,face);

                float cosTheta = dot(sampleDir,N);

                if(cosTheta>0.)
                {
                    float tmp = 1.f + pos.x * pos.y + pos.y * pos.y;
                    float weight = 4.f * cosTheta/(sqrt(tmp) * tmp); //Based on solid angle https://www.rorydriscoll.com/2012/01/15/cubemap-texel-solid-angle/

                    irradiance += EnvMap.SampleLevel(sampleDir,0).xyz * weight;
                    sampleCount += weight;
                }
            }
        }
    }

    return irradiance/sampleCount;
}

float3 SpecularFilteredIBL(SamplerCube EnvMap, float3 N, float Roughness, uint SampleCount)
{
    uint width, height;
    EnvMap.GetDimensions(width,height);

    float3 c = 0.;
    float weight = 0.0f;

    for(uint i = 0; i<SampleCount;i++)
    {
            float4 importanceSample = GetDGGXImportanceSample(i, N, Roughness, SampleCount);
            float3 H = importanceSample.xyz;
            float pdf = importanceSample.w;

            float lod = ComputeLod(pdf,width,height, SampleCount); //LOD here is playing the role of the PDF divide
            //GGX Distribution
            float3 V = N;
            float3 L = normalize(reflect(-V,H));
            float NdL = dot(N,L);
            if(NdL>0)
            {
                if(Roughness == 0.0) lod = 0.0;

                float3 sampleColor = EnvMap.SampleLevel(L,lod).xyz;

                c += sampleColor*NdL;
                weight += NdL;
            }
    }

    c /= (weight != 0.0f) ? weight : float(SampleCount);
    return c;
}


[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 ThreadID:SV_DispatchThreadID, uint3 GroupID: SV_GroupID)
{
    float2 uv = (ThreadID.xy)/float2(material.outputResolution);
    uv = uv * 2 - 1.;

    uint face = GroupID.z;

    if(material.roughness == 0)
    {
            float3 c = material.input.SampleLevel(CubeCoordToDir(uv.x,uv.y,face),0).xyz;
            material.output[uint3(ThreadID.xy,face)] = float4(c,1.);
            return;
    }

    float3 c =  SpecularFilteredIBL(material.input, CubeCoordToDir(uv.x,uv.y,face),material.roughness,material.sampleCount);

    material.output[uint3(ThreadID.xy,face)] = float4(c, 1.);
}


[shader("compute")]
[numthreads(32, 32, 1)]
void Kernel_DiffuseIBL(uint3 ThreadID:SV_DispatchThreadID, uint3 GroupID: SV_GroupID)
{
    float2 uv = (ThreadID.xy)/float2(materialDiffuse.outputResolution);
    uv = uv * 2 - 1.;

    uint face = GroupID.z;
    float3 cubeDir =  CubeCoordToDir(uv.x,uv.y,face);

    float3 c =  DiffuseFilteredIBL_ImportanceSampled(materialDiffuse.input,cubeDir, materialDiffuse.sampleCount);

    materialDiffuse.output[uint3(ThreadID.xy,face)] = float4(c, 1.);
}
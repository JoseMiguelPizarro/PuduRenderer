import PuduCoreModule;
import PuduRandomModule;
import "Lib/PuduLightingModule.slang";


struct Input
{
    RWTexture2DArray<float4> output;
    Sampler2D input;
    float roughness;
    float sampleCount;
}

ParameterBlock<Input> material;


// mipmap filtered samples (GPU Gems 3, 20.4)
float ComputeLod(float PDF, uint Width, uint Height, uint SampleCount) {
  float w = float(Width);
  float h = float(Height);
  float sampleCountF = float(SampleCount);
  return 0.5 * log2( 6.0 * w * h / (sampleCountF * PDF));
}

float4 GetImportanceSample(uint SampleIndex, float3 N, float Roughness, uint SampleCount)
{
    float2 xi = HammersleyHemisphere(SampleIndex,SampleCount);
    MicrofacetDistributionSample importanceSample = GGX(xi,Roughness);

    float3 directionTS = normalize(
        float3(
        importanceSample.sinTheta * cos(importanceSample.phi),
        importanceSample.sinTheta * sin(importanceSample.phi),
        importanceSample.cosTheta));

    float3x3 TBN = GenerateTBN(N);
    float3 dir = mul(TBN, directionTS);

    return float4(dir,importanceSample.pdf);
}

float3 DiffuseFilteredIBL(Sampler2D EnvMap, float3 N, float Roughness, uint SampleCount)
{
    uint width, height;
    EnvMap.GetDimensions(width,height);

    float3 c = 0.;
    float weight = 0.0f;

    for(uint i = 0; i<SampleCount;i++)
    {
            float4 importanceSample = GetImportanceSample(i, N, Roughness, SampleCount);
            float3 H = importanceSample.xyz;
            float pdf = importanceSample.w;

            float lod = ComputeLod(pdf,width,height, SampleCount);
            //GGX Distribution
            float3 V = N;
            float3 L = normalize(reflect(-V,H));
            float NdL = dot(N,L);
            if(NdL>0)
            {
                if(Roughness == 0.0) lod = 0.0;

                float3 sampleColor = EnvMap.SampleLevel(DirToLongitudeLatitude(L),lod).xyz;

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
    uint width, height, arraySize;
    material.output.GetDimensions(width, height, arraySize);

    float2 uv = (ThreadID.xy)/float2(width,height);
    uv = uv * 2 - 1.;

    uint face = GroupID.z;

    float3 n = CubeCoordToDir(uv.x,uv.y,face);
    float3 c =  DiffuseFilteredIBL(material.input, CubeCoordToDir(uv.x,uv.y,face),0.2,1024);

    n = material.input.SampleLevel(DirToLongitudeLatitude(n),0).xyz;
    material.output[uint3(ThreadID.xy,face)] = float4(c, 1.);
}

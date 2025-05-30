import PuduCoreModule;
import PuduLightingModule;


struct Input
{
    RWTexture2D<float4> output;
    Sampler2D input;
    uint sampleCount;
}

ParameterBlock<Input> material;

[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 ThreadID:SV_DispatchThreadID, uint3 GroupID: SV_GroupID)
{
    uint width, height, arraySize;
    material.output.GetDimensions(width, height, arraySize);

    float2 uv = (ThreadID.xy)/float2(width,height);
    uv.y = 1. - uv.y;

    float3 brdf = BRDF(uv.x,uv.y);

    output[ThreadID.xy]  = float4(brdf.xy,0,1);
}

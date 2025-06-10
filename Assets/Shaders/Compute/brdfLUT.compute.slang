import PuduCoreModule;
import "Lib/PuduLightingModule";


struct Input
{
    RWTexture2D<float4> output;
    uint sampleCount;
}

ParameterBlock<Input> material;

[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 ThreadID:SV_DispatchThreadID, uint3 GroupID: SV_GroupID)
{
    uint width, height;
    material.output.GetDimensions(width, height);

    float2 uv = (ThreadID.xy +.5)/float2(width,height);
    uv.y = 1. - uv.y;

    float3 brdf = BRDF(uv.x,uv.y);

    float v25 = step(uv.y,0.25);
    float v50 = step(uv.y,0.5);
    float v75 = step(uv.y,.75);

    uint2 outCoords = {ThreadID.x, height - ThreadID.y - 1};

    float3 c = v25*float3(1,0,0) + (v50 - v25)*float3(0,1,0) + (v75 - v50)*float3(0,0,1) + (1. - v75)*float3(1,1,1);
    material.output[outCoords]  = float4(brdf,1);
}



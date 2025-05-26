import PuduCoreModule;


struct Input
{
    RWTexture2DArray<float4> output;
    Sampler2D input;
}

ParameterBlock<Input> material;


[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gId: SV_GroupID)
{
 //   material.output[gId.xy] = material.input[gId.xy] * float4(1,0.2,0.3,1.);

    uint size = 512;

    float2 uv = (gId.xy)/float(size);
    uv = uv * 2 - 1.;


    [ForceUnroll]
    for(uint i =0;i<6;i++)
    {
        float2 longLat = DirToLongitudeLatitude(CubeCoordToDir(uv.x,uv.y,i));
        material.output[uint3(gId.xy,i)] = material.input.SampleLevel(longLat,0.);
    }
}

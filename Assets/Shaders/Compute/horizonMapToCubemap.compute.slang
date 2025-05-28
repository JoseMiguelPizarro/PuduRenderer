import PuduCoreModule;


struct Input
{
    RWTexture2DArray<float4> output;
    Sampler2D input;
}

ParameterBlock<Input> material;


[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 threadId:SV_DispatchThreadID, uint3 gId: SV_GroupID)
{
 //   material.output[gId.xy] = material.input[gId.xy] * float4(1,0.2,0.3,1.);

    uint width, height, arraySize;
    material.output.GetDimensions(width, height, arraySize);

    float2 uv = (threadId.xy)/float2(width,height);
    uv = uv * 2 - 1.;

    float2 longLat = DirToLongitudeLatitude(CubeCoordToDir(uv.x,uv.y,gId.z));
    material.output[uint3(threadId.xy,gId.z)] = material.input.SampleLevel(longLat,0.);
}

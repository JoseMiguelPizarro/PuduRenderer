import PuduCoreModule;


struct Input
{
    RWTexture2DArray<float4> output;
    Sampler2D input;
}

ParameterBlock<Input> material;


[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 ThreadID:SV_DispatchThreadID, uint3 GroupID: SV_GroupID)
{
    uint width, height, arraySize;
    material.output.GetDimensions(width, height, arraySize);

    float2 uv = (ThreadID.xy)/float2(width,height);
    uv = uv * 2 - 1.;

    uint face = GroupID.z;
    float2 longLat = DirToLongitudeLatitude(CubeCoordToDir(uv.x,uv.y,face));
    material.output[uint3(ThreadID.xy,face)] = material.input.SampleLevel(longLat,0.);
}


struct TestData
{
    RWTexture2D<float4> test;
}

ParameterBlock<TestData> material;


[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_GroupID)
{
    material.test[threadId.xy] = float4(1,0,0,1);
}

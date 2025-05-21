
struct TestData
{
    RWTexture2D<float4> test;
}

ParameterBlock<TestData> material;


[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gId: SV_GroupID)
{
    material.test[gId.xy] = float4(gId.xy/1024.,0,1);
}

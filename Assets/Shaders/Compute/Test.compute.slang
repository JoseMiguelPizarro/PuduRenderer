struct TestData
{
    RWTexture2D<float4> output;
    Sampler2D input;
}

ParameterBlock<TestData> material;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gId: SV_GroupID)
{
 //   material.output[gId.xy] = material.input[gId.xy] * float4(1,0.2,0.3,1.);
    material.output[gId.xy] = material.input.SampleLevel(gId.xy / float2(4096,2048),0);
}

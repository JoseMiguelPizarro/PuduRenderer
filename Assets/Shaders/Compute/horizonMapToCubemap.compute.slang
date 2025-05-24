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

    float4 colors[] =
    {float4(1,0,0,1),float4(0,1,0,1),float4(0,0,1,1),float4(1,1,0,1),float4(0,1,1,1),
        float4(1,0,1,1)};

    [ForceUnroll]
    for(uint i =0;i<6;i++)
    {
        material.output[uint3(gId.xy,i)] = colors[i];
    }
}

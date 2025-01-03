#include "Lib/DefaultVertexInput.hlsl"

struct PostProcessingData
{
    Sampler2D colorTex;
}

ParameterBlock<PostProcessingData> Data;

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
    Sampler2D tex = Data.colorTex;

    float r = tex.Sample(input.TexCoord.xy).x;
    float g = tex.Sample(input.TexCoord.xy + float2(0.01, 0.01)).y;
    float b = tex.Sample(input.TexCoord.xy + float2(0.02, 0.015)).z;

    return float4(r, g, b, 0);
}

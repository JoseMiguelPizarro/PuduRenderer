#include "Lib/DefaultVertexInput.hlsl"

[[vk::binding(32, 0)]]Sampler2D global_textures[];

[shader("pixel")]
float4 main(VSOut input):SV_TARGET {
     Sampler2D tex = global_textures[NonUniformResourceIndex(5)];

    float r = tex.Sample(input.TexCoord.xy).x;
    float g = tex.Sample(input.TexCoord.xy + float2(0.01,0.01)).y;
    float b = tex.Sample(input.TexCoord.xy + float2(0.02,0.015)).z;

    return float4(r,g,b,0);
}

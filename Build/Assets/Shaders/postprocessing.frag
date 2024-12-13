#include "Lib/DefaultVertexInput.hlsl"

[[vk::binding(32, 0)]]Sampler2D global_textures[];

[shader("pixel")]
float4 main(VSOut input):SV_TARGET {
     Sampler2D tex = global_textures[NonUniformResourceIndex(5)];

    return float4(input.TexCoord.xy,0,0);
}

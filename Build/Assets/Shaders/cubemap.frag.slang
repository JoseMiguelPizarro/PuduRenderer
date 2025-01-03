// Bindless support
//[[vk::binding(0, 0)]] SamplerCube cubeMap;
#include "Lib/DefaultVertexInput.hlsl"
import PuduGraphics;

SamplerCube testText;

float linearDepth(float d, float near, float far)
{
    return near * far / (far + d * (near - far));
}

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
    uint id = UBO.materialId;

    float3 coord = input.Normal.xyz;
    // float4 col = cubeMap.Sample(coord);
    Sampler2D tex = GetGlobalTexture(id);

    float4 baseColor = testText.Sample(coord) + saturate(tex.Sample(input.TexCoord.xy)) * 0.001;
    // col.xyz = float3(input.Color);

    float4 col = baseColor;
    // col.xyz = input.Normal.xyz;

    return col;
}

// Bindless support
//[[vk::binding(0, 0)]] SamplerCube cubeMap;
#include "PuduGraphics.hlsl"
#include "Lib/DefaultVertexInput.hlsl"

BIND(0) SamplerCube testText;
CONSTANTS(UniformBufferObject) ubo;


float linearDepth(float d, float near, float far)
{
return near * far / (far + d * (near - far));
}

[shader("pixel")]
float4 main(VSOut input): SV_TARGET {
    uint id = ubo.materialId;

    float3 coord = input.Normal.xyz;
    //float4 col = cubeMap.Sample(coord);
    Sampler2D tex = GET_GLOBAL_TEXTURE(id);

    float4 baseColor = testText.Sample(coord) + saturate(tex.Sample(input.TexCoord.xy))*0.001;
    //col.xyz = float3(input.Color);

    float4 col = baseColor;
//col.xyz = input.Normal.xyz;


    return col;
}

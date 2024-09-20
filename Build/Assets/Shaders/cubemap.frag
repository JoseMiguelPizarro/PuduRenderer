// Bindless support
//[[vk::binding(0, 0)]] SamplerCube cubeMap;
#include "PuduGraphics.hlsl"

BIND(0) Sampler2D testText;
CONSTANTS(UniformBufferObject) ubo;

struct VSOut
{
float4 PositionCS: SV_POSITION;
float4 Color: COLOR;
float4 TexCoord: TEXCOORD;
float4 Normal: NORMAL;
float4 ShadowCoords: TEXCOORD1;
};

float linearDepth(float d, float near, float far)
{
return near * far / (far + d * (near - far));
}

vec4 GetLighting(vec3 normal, vec3 lightDirection)
{
float ndl = dot(normal, lightDirection) * 0.5 + 0.5;
//    return vec4(ndl);

return mix(vec4(1), vec4(0.54, 0.95, 1, 0), 1 - ndl);
}

[shader("pixel")]
float4 main(VSOut input): SV_TARGET {
    uint id = ubo.materialId;

    float3 coord = input.Normal.xyz;
    
    //float4 col = cubeMap.Sample(coord);
    Sampler2D tex = GET_GLOBAL_TEXTURE(id);
   // float4 base_color = tex.Sample(input.TexCoord.xy) + cubeMap.Sample(float3(0,0,0));

    float4 baseColor = testText.Sample(input.TexCoord.xy) + tex.Sample(input.TexCoord.xy);
    float4 col = baseColor;
    
    return col;
}

// Bindless support
#include "Lib/DefaultVertexInput.hlsl"
#include "Lib/Lighting.hlsl"

[[vk::binding(32, 0)]]
Sampler2D global_textures[];

[[vk::binding(0, 1)]]
ConstantBuffer<LightBuffer> lightingBuffer;

#ifndef SHADOWMAP
#define SHADOWMAP global_textures[NonUniformResourceIndex(4)]
#endif

[[vk::push_constant]]
ConstantBuffer<UniformBufferObject> ubo;

float linearDepth(float d, float near, float far)
{
    return near * far / (far + d * (near - far));
}

float4 GetLighting(float3 normal, float3 lightDirection)
{
    float ndl = dot(normal, lightDirection) * 0.5 + 0.5;
    //    return vec4(ndl);

    return lerp(1., float4(0.54, 0.95, 1, 0), 1 - ndl);
}

float TextureProj(float4 shadowCoord, float2 off)
{
    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
    {
        Sampler2D tex = SHADOWMAP;
        float dist = tex.Sample(shadowCoord.xy + off).x;
        if (shadowCoord.w > 0.0 && dist < shadowCoord.z)
        {
            shadow = 0.0f;
        }
    }
    return shadow;
}

float FilterPCF(float4 sc)
{
    int2 texDim;
    SHADOWMAP.GetDimensions(texDim.x, texDim.y);
    float scale = 1.5;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    int range = 1;

    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            shadowFactor += TextureProj(sc, float2(dx * x, dy * y));
            count++;
        }
    }
    return shadowFactor / count;
}

[shader("pixel")]
float4 main(VSOut input): SV_TARGET
{
    uint id = ubo.materialId;
    Sampler2D tex = global_textures[NonUniformResourceIndex(id)];
    float4 base_colour = tex.Sample(input.TexCoord.xy);
    float2 suv = (input.PositionCS * 0.5 + 0.5).xy; 
    
    // Perspective Devide shadow to map 0-1
    float shadow = saturate(FilterPCF(input.ShadowCoords / input.ShadowCoords.w) + 0.5);
    float4 col = pow(base_colour, float4(1.0 / 2.2)) * GetLighting(normalize(input.Normal.xyz), lightingBuffer.lightDirection.xyz);
    col = lerp(col, float4(0.2, 0.2, 0.3, 1.0), 1 - shadow);

    float3 viewPos = float3(ubo.view._14, ubo.view._24, ubo.view._34);
    float d = 1. - saturate(distance(input.PosWS.xyz, viewPos)/80.f);
    return lerp(float4(0.9,.4,0.6,0.), col, d);

  //  return SHADOWMAP.Sample(suv);
    //return SHADOWMAP.Sample((input.ShadowCoords / input.ShadowCoords.w).xy).xxxx;
    // return shadow;
    return col;
}

// Bindless support
#include "Lib/DefaultVertexInput.hlsl"
#include "lib/Shadowmap.slang"

import PuduGraphics;

struct StandardMaterial
{
    Sampler2D baseColorTex;
    Sampler2D normalTex;
}

ParameterBlock<StandardMaterial> material;

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

[shader("vertex")]
VSOut vertexMain(VertexInput input)
{
    VSOut output = (VSOut)0.0;
    output.PositionCS = mul(UBO.proj, mul(UBO.view, mul(UBO.model, float4(input.Position, 1.0))));
    output.PosWS = mul(UBO.model, float4(input.Position, 1));
    output.Color = float4(input.Color, 1.0);
    output.TexCoord = float4(input.TexCoord, 0, 0);
    output.Normal = float4(input.Normal, 0.);
    output.PosOS = float4(input.Position, 0.);

    LightBuffer lightingBuffer = GLOBALS.lightingBuffer;
    output.ShadowCoords = GetShadowCoord(lightingBuffer.shadowMatrix, lightingBuffer.lightMatrix, mul(UBO.model, float4(input.Position, 1.)));
    // mul(biasMat, mul(lightingBuffer.shadowMatrix, mul(lightingBuffer.lightMatrix, mul(UBO.model, float4(input.Position, 1.0)))));
    //  output.ShadowCoords = mul(float4(input.Position, 1.0), mul(ubo.model, mul(lightingBuffer.lightMatrix, mul(lightingBuffer.shadowMatrix, biasMat))));

    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
    uint id = UBO.materialId;
    Sampler2D tex = material.baseColorTex;
    float4 base_colour = tex.Sample(input.TexCoord.xy);
    float2 suv = (input.PositionCS * 0.5 + 0.5).xy;

    // Perspective Devide shadow to map 0-1
    float shadow = saturate(FilterPCF(input.ShadowCoords / input.ShadowCoords.w, GLOBALS.shadowMap) + 0.5);
    float4 col = pow(base_colour, float4(1.0 / 2.2)) * GetLighting(normalize(input.Normal.xyz), GLOBALS.lightingBuffer.lightDirection.xyz);
    col = lerp(col, float4(0.2, 0.2, 0.3, 1.0), 1 - shadow);

    float3 viewPos = float3(UBO.view._14, UBO.view._24, UBO.view._34);
    float d = 1. - saturate(distance(input.PosWS.xyz, viewPos) / 80.f);
    return lerp(float4(0.9, .4, 0.6, 0.), col, d);
}

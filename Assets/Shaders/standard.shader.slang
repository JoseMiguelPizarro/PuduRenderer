// Bindless support
#include "Lib/DefaultVertexInput.hlsl"
#include "lib/Shadowmap.slang"

import PuduGraphicsModule;


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
    output.PosCS = mul(GetProjectionMatrix(), mul(GetViewMatrix(), mul(UBO.model, float4(input.Position, 1.0))));
    output.PosWS = mul(UBO.model, float4(input.Position, 1));
    output.Color = float4(input.Color, 1.0);
    output.TexCoord = float4(input.TexCoord, 0, 0);
    output.Normal = float4(input.Normal, 0.);
    output.Tangent = input.Tangent;
    output.PosOS = float4(input.Position, 0.);

    LightBuffer lightingBuffer = GLOBALS.lightingBuffer;
    output.ShadowCoords = GetShadowCoord(lightingBuffer.shadowMatrix, lightingBuffer.lightMatrix, output.PosWS);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
    uint id = UBO.materialId;
    Sampler2D tex = material.baseColorTex;
    float4 base_colour = tex.Sample(input.TexCoord.xy) * input.Color;
    float2 suv = GetPixelScreenPosition(input.PosCS);

    float shadow = saturate(FilterPCF(input.ShadowCoords / input.ShadowCoords.w, GLOBALS.shadowMap) + 0.5);
    float4 col = pow(base_colour, float4(1.0 / 2.2)) * GetLighting(normalize(input.Normal.xyz), GLOBALS.lightingBuffer.lightDirection.xyz);
    col = lerp(col, float4(0.2, 0.2, 0.3, 1.0), 1 - shadow);

    var viewMatrix = GetViewMatrix();
    float3 viewPos = float3(viewMatrix._14, viewMatrix._24, viewMatrix._34);
    float d = 1. - saturate(distance(input.PosWS.xyz, viewPos) / 80.f);

    float3 normalColor = GLOBALS.normalBuffer.Sample(suv).xyz;
    float depth = GLOBALS.depthBuffer.Sample(suv).x;
    float depthColor = LinearizeDepth(GLOBALS.depthBuffer.Sample(suv).x,GLOBALS.constants.nearPlane,GLOBALS.constants.farPlane);

    col = lerp(float4(0.9, .4, 0.6, 0.), col, d);

    col.a = 0.5;
    return col;
}

#include "Lib/DefaultVertexInput.hlsl"
#include "lib/Shadowmap.slang"

import PuduGraphicsModule;
import "Lib/LightingModule.slang";

struct StandardMaterial
{
    Sampler2D albedoTex;
    Sampler2D normalTex;
    Sampler2D roughnessTex;
    Sampler2D metallicTex;
    SamplerCube skybox; //Placeholder for environment
}
ParameterBlock<StandardMaterial> material;

// struct SkyboxAmbientSampler:IDiffuseAmbientSampler
// {
//     SamplerCube m_skybox;
//
//     float3 Sample(float3 n)
//     {
//         return m_skybox.Sample(n).xyz;
//     }
// }

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
    float4 col = 0;
    float3 albedo =material.albedoTex.Sample(input.TexCoord.xy).rgb;
    float shadow = saturate(FilterPCF(input.ShadowCoords / input.ShadowCoords.w, GLOBALS.shadowMap) + 0.5);

    float3 normalTS = DecodeNormal(material.normalTex.Sample(input.TexCoord.xy).xy);
    float3 tangent = normalize( input.Tangent.xyz);
    float3 normal = normalize(input.Normal.xyz);
    float3 bitangent = cross(normal,tangent);

    float3 normalWS = TangentToWorld(normalTS,tangent,bitangent,normal) * input.Tangent.w;

//     SkyboxAmbientSampler ambientSampler;
//     ambientSampler.m_skybox = material.skybox;

    SurfaceInfo m = {};
    m.albedo = albedo;
    m.lightDir = normalize(GLOBALS.lightingBuffer.lightDirection.xyz);
    m.lightColor = float3(1,1,1);
    m.specularColor = float3(1,1,1);
    m.normal = normalWS;
    m.glossiness =1. - material.roughnessTex.Sample(input.TexCoord.xy).x;
    m.viewDir = GetViewDirection(input.PosWS.xyz);
    m.ambientSampler = material.skybox;

    col.rgb = StandardSurface(m);
    return col;
}
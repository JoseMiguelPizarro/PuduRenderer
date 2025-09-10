#include "Lib/DefaultVertexInput.hlsl"

import PuduCoreModule;
import PuduGraphicsModule;
#include "lib/PuduShadowModule.slang"
#include "lib/PuduLightingModule.slang"


struct StandardMaterial
{
    Sampler2D albedoTex;
    Sampler2D normalTex;
    Sampler2D metallicRoughnessTex;
    Sampler2D heightTex;
    Sampler2D emissiveTex;
    float3 F0 = {0.04f, 0.04f, 0.04f};
    float3 F90 = {0.5f, 0.5f, 0.5f};
    float parallax = 0;
}

ParameterBlock<StandardMaterial> material;

[shader("vertex")]
VSOut vertexMain(VertexInput input)
{
    VSOut output = (VSOut)0.0;
    output.PosCS = mul(GetProjectionMatrix(), mul(GetViewMatrix(), mul(UBO.model, float4(input.Position, 1.0))));
    output.PosWS = mul(UBO.model, float4(input.Position, 1));
    output.Color =input.Color;
    output.TexCoord = float4(input.TexCoord, 0, 0);
    output.Normal = mul(UBO.model, float4(input.Normal, 0.));
    output.Tangent = mul(UBO.model,input.Tangent);
    output.PosOS = float4(input.Position, 0.);

    let directionalLight = LIGHTING.lights.directionalLight;
    output.ShadowCoords = GetShadowCoord(directionalLight.shadowMatrix, directionalLight.lightMatrix, output.PosWS);

    return output;
}

[
shader("fragment"),
ColorMask(ColorMask.All)
]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
    float4 col = 0;

    float shadow = saturate(FilterPCF(input.ShadowCoords, LIGHTING.shadowMap));


    float3 viewDir = GetViewDirection(input.PosWS.xyz);
    float3 tangent = normalize( input.Tangent.xyz);
    float3 normal = normalize(input.Normal.xyz);
    float3 bitangent = cross(normal,tangent) * input.Tangent.w;
    float height = material.heightTex.Sample(input.TexCoord.xy).x;

    float3 tangentView = WorldToTangent(-viewDir,tangent,bitangent,normal);
    let directionalLight = LIGHTING.lights.directionalLight;

    float3 lightDirTS = WorldToTangent(directionalLight.lightDirection.xyz,tangent,bitangent,normal);
    float2 uv = input.TexCoord.xy;

    if(material.parallax>0)
    {
        let pom = ParallaxOcclusionMapping_Shadow(material.heightTex,input.TexCoord.xy,tangentView,lightDirTS,0.08);
        shadow *= pom.shadow;
        uv = pom.uv;
     }

    float3 albedo = material.albedoTex.Sample(uv).rgb * input.Color.rgb;
    float3 normalTS = DecodeNormal(material.normalTex.Sample(uv).xy);

    float3 normalWS = TangentToWorld(normalTS,tangent,bitangent,normal);
    SurfaceInfo m = {};
    float2 metallicRoughness = material.metallicRoughnessTex.Sample(uv).bg; // MetallicRoughnessChannels according to gltf2.0 specs
    m.Albedo = albedo;
    m.SpecularColor = float3(1,1,1);
    m.Normal = normalWS;
    m.PosWS = input.PosWS.xyz;
    m.Metallic = metallicRoughness.x;
    m.Roughness = metallicRoughness.y;
    m.ViewDir = viewDir;
    m.ShadowAttenuation = shadow;
    m.F0 = material.F0.xyz;
    m.F90 = material.F90.xyz;
    m.Emissive = material.emissiveTex.Sample(uv).xyz;

    col.xyz = StandardSurface(m);
    col.a = 1;

    return col;
}
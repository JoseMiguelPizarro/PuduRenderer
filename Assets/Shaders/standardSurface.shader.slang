#include "Lib/DefaultVertexInput.hlsl"
#include "lib/Shadowmap.slang"


import PuduGraphicsModule;
import "Lib/LightingModule.slang";

[shader("vertex")]
VSOut vertexMain(VertexInput input)
{
    VSOut output = (VSOut)0.0;
    output.PosCS = mul(GetProjectionMatrix(), mul(GetViewMatrix(), mul(UBO.model, float4(input.Position, 1.0))));
    output.PosWS = mul(UBO.model, float4(input.Position, 1));
    output.Color = float4(input.Color, 1.0);
    output.TexCoord = float4(input.TexCoord, 0, 0);
    output.Normal = float4(input.Normal, 0.);
    output.Tangent = float4(input.Tangent,0.);
    output.PosOS = float4(input.Position, 0.);

    LightBuffer lightingBuffer = GLOBALS.lightingBuffer;
    output.ShadowCoords = GetShadowCoord(lightingBuffer.shadowMatrix, lightingBuffer.lightMatrix, output.PosWS);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
    float4 col = float4(.8,.1,.1,0);
    float shadow = saturate(FilterPCF(input.ShadowCoords / input.ShadowCoords.w, GLOBALS.shadowMap) + 0.5);

    SurfaceMaterial m = {};
    m.albedo = col.rgb;
    m.lightDir = normalize(GLOBALS.lightingBuffer.lightDirection.xyz);
    m.lightColor = float3(1,1,.1);
    m.specularColor = float3(1,1,1);
    m.normal = normalize(input.Normal.xyz);
    m.viewDir = GetViewDirection(input.PosWS.xyz);
    col.rgb = StandardSurface(m);

    return col;
}
// Bindless support
#include "Lib/DefaultVertexInput.hlsl"
#include "lib/Shadowmap.slang"

import PuduGraphicsModule;

struct SkyboxMaterial
{
    SamplerCube skyboxTex;
}

ParameterBlock<SkyboxMaterial> material;

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

    output.viewDir = output.PosWS.xyz - GLOBALS.constants.cameraPosWS.xyz;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
    var viewDir = normalize(input.viewDir);

    var col = material.skyboxTex.Sample(viewDir);

    return col;
}

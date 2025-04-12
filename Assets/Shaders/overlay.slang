#include "Lib/DefaultVertexInput.hlsl"
#include "Lib/Lighting.hlsl"

import PuduGraphicsModule;

[shader("vertex")]
VSOut vertexMain(VertexInput input)
{
    VSOut output = (VSOut)0.0;

    float3 forward = float3(GetViewMatrix()[0][2], GetViewMatrix()[1][2], GetViewMatrix()[2][2]);
    float3 right = float3(GetViewMatrix()[0][0], GetViewMatrix()[1][0], GetViewMatrix()[2][0]);
    float3 up = float3(GetViewMatrix()[0][1], GetViewMatrix()[1][1], GetViewMatrix()[2][1]);

    float4x4 billBoard = {
        float4(right, 0),
        float4(up, 0),
        float4(forward, 0),
        float4(0, 0, 0, 1)
    };

    output.PosCS = mul(GetProjectionMatrix(), mul(GetViewMatrix(), mul(UBO.model, float4(input.Position, 1.0))));
    output.PosWS = mul(UBO.model, float4(input.Position, 1));
    output.Color = float4(input.Color, 1.0);
    output.TexCoord = float4(input.TexCoord, 0, 0);
    output.Normal = float4(input.Normal, 0.);
    output.PosOS = float4(input.Position, 0.);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_Target
{
    return input.Color;
}

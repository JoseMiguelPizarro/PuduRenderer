#include "Lib/DefaultVertexInput.hlsl"

import PuduGraphics;

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
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_Target
{
    return float4(1, 0, 0, .5);
}

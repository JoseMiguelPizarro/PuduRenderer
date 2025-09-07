#include "Lib/DefaultVertexInput.hlsl"

import PuduGraphicsModule;
import PuduCoreModule;


[shader("vertex")]
VSOut vertexMain(VertexInput input)
{
    VSOut output = (VSOut)0.0;
    output.PosCS = mul(GetProjectionMatrix(), mul(GetViewMatrix(), mul(UBO.model, float4(input.Position, 1.0))));
    output.PosWS = mul(UBO.model, float4(input.Position, 1));
    output.Color = input.Color;
    output.TexCoord = float4(input.TexCoord, 0, 0);
    output.Normal = mul(UBO.model, float4(input.Normal, 0.));
    output.Tangent = mul(UBO.model,input.Tangent);
    output.PosOS = float4(input.Position, 0.);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
   float2 screenUV = GetPixelScreenPosition(input.PosCS);

   screenUV.xy *= 12;
   float id = floor(screenUV.y);

   float check =floor((screenUV.x+ id%2)%2);

   return lerp( float4(1,0,1,1), GLOBALS.pudu.Sample(frac(screenUV)),check);
}
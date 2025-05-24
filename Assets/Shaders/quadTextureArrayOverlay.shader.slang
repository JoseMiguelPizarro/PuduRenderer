#include "Lib/DefaultVertexInput.hlsl"

import PuduGraphicsModule;

//Coordinates and size in NDC
struct OverlayData{
    float x;
    float y;
    float width;
    float height;
    };

struct Material
{
    Sampler2DArray texture;
    float index;
};

ParameterBlock<OverlayData> overlayData;
ParameterBlock<Material> material;

[shader("vertex")]
VSOut vertexMain(VertexInput input)
{
    VSOut output = (VSOut)0.0;

    float2 pos = float2(input.Position.xz) + float2(0.5, 0.5f); //move pivot to bottom left corner;

//Convert screen coordinates to NDC
    pos = float2(overlayData.x,overlayData.y)*2 -1 + pos*float2(overlayData.width,overlayData.height)*2;

    float4 posCS = float4(pos,0, 1);

    output.PosCS = posCS;
    output.Color = float4(input.Color, 1.0);
    output.TexCoord = float4(input.TexCoord, 0, 0);
    output.Normal = float4(input.Normal, 0.);
    output.PosOS = float4(input.Position, 0.);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOut input) : SV_Target
{
    return material.texture.Sample(float3(input.TexCoord.xy,material.index));
}

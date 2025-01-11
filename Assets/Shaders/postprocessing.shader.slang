#include "Lib/DefaultVertexInput.hlsl"

struct PostProcessingData
{
    Sampler2D colorTex;
}

ParameterBlock<PostProcessingData> Data;

static float4 pos[] = {
    float4(-1, -1, 0, 0),
    float4(-1, 1, 0, 0),
    float4(1, 1, 0, 0),
    float4(1, -1, 0, 0)
};

[shader("vertex")]
VSOut vertexMain(VertexInput in, uint vertexId: SV_VertexID)
{
    VSOut output = (VSOut)0.0;
    output.PositionCS = float4(pos[vertexId].xyz, 1);
    output.TexCoord = float4(in.TexCoord, 0, 0);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
    Sampler2D tex = Data.colorTex;

    float r = tex.Sample(input.TexCoord.xy).x;
    float g = tex.Sample(input.TexCoord.xy + float2(0.005, 0.005)).y;
    float b = tex.Sample(input.TexCoord.xy + float2(0.01, 0.01)).z;

    return float4(r, g, b, 0);
}

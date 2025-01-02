#include "Lib/DefaultVertexInput.hlsl"

static float4 pos[] = {
    float4(-1, -1, 0, 0),
    float4(-1, 1, 0, 0),
    float4(1, 1, 0, 0),
    float4(1, -1, 0, 0)
};

[shader("vertex")]
VSOut main(VertexInput in, uint vertexId: SV_VertexID)
{
    VSOut output = (VSOut)0.0;
    output.PositionCS = float4(pos[vertexId].xyz, 1);
    output.TexCoord = float4(in.TexCoord, 0, 0);

    return output;
}

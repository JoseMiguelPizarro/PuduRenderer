#include "Lib/DefaultVertexInput.hlsl"

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
    output.PosCS = float4(pos[vertexId].xyz, 1);
    output.TexCoord = float4(in.TexCoord, 0, 0);

    return output;
}

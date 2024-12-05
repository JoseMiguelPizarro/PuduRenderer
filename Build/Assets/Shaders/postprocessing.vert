
struct VSOut
{
    float4 PositionCS: SV_POSITION;
};

float4 pos[] = {
float4(-1,-1,0,0),
float4(-1,1,0,0),
float4(1,1,0,0),
float4(1,-1,0,0)
};

[shader("vertex")]
VSOut main(uint vertexId:SV_VertexID) {
    VSOut output = (VSOut)0.0;
    output.PositionCS = pos[vertexId];
        
    return output;
}

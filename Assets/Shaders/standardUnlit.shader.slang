import PuduCoreModule;
import PuduGraphicsModule;


struct VertexInput
{
    [POSITION] float3 position;
};

struct VertexOutput
{
    float4 positionCS:SV_Position;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput Input)
{
    VertexOutput output = {};
    output.positionCS = GetPositionCS(UBO.model, Input.position);

    return output;
};

[shader("fragment")]
float4 fragmentMain(VertexOutput Input):SV_TARGET
{
return float4(1,0,0,1);
};
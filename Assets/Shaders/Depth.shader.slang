import PuduCoreModule;
import PuduGraphicsModule;

struct VertexInput
{
 	[POSITION] float3 Position;
};

[shader("vertex")]
float4 vertexMain(VertexInput input):SV_POSITION
{
    return GetPositionCS(UBO.model, input.Position);
};

[shader("fragment")]
float4 fragmentMain(float4 input)
    : SV_Target
{
    return 1;
}

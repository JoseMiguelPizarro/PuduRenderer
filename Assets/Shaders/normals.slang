import PuduCoreModule;
import PuduGraphicsModule;

struct VertexInput
{
    [POSITION] 	float3 Position;
    [NORMAL] 	float3 Normal;
    [TANGENT] 	float4 Tangent;
};

struct VSOut{
    float4 PosCS;
    float4 Normal;
}

[shader("vertex")]
VSOut vertexMain(VertexInput input)
{
    VSOut output = (VSOut)0.0;
    output.PosCS = mul(GetProjectionMatrix(), mul(GetViewMatrix(), mul(UBO.model, float4(input.Position, 1.0))));
    output.Normal = float4(input.Normal, 0.);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_Target
{
    return input.Normal * 0.5 + 0.5; //TODO: SAMPLE NORMAL TEX
}

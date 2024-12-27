#include "lib/Lighting.hlsl"
#include "lib/grass.slang"

static float3 vertices[] = {
    float3(-0.5, 0, 0),
    float3(0.0, 0.5, 0),
    float3(0.5, 0, 0)
};

ParameterBlock<GrassData> Data;
[[vk::push_constant]]
ConstantBuffer<UniformBufferObject> ubo;

[shader("vertex")]
float4 main(uint id: SV_InstanceID, uint vertexId: SV_VertexID)
    : SV_Position
{
    var grass = Data.GrassPos[id];
    var pos = vertices[vertexId] * 10. + float3(grass.pos.x, 0, grass.pos.y);

    return mul(ubo.proj, mul(ubo.view, float4(pos, 1)));
}

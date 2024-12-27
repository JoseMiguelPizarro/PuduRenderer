#include "lib/Lighting.hlsl"
#include "lib/grass.slang"

static float3 vertices[] = {
    float3(-0.5, 0, 0),
    float3(0.5, 0.5, 0),
    float3(0.5, 0, 0)
};

[[vk::push_constant]]
ConstantBuffer<UniformBufferObject> ubo;

ParameterBlock<GrassData> Data;

[shader("fragment")]
float4 main(float4 clipPos: SV_Position)
    : SV_Target
{
    return float4(1 * Data.GrassPos[0].pos.x, 0, 0, 1);
}

#include "lib/Lighting.hlsl"
#include "lib/Shadowmap.slang"
#include "lib/grass.slang"

[[vk::push_constant]]
ConstantBuffer<UniformBufferObject> ubo;

ParameterBlock<GrassData> Data;

[shader("fragment")]
float4 main(GrassVertexOutput input)
    : SV_Target
{
    return input.color;
}

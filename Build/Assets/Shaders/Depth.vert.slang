#include "Lib/Lighting.hlsl"
#include "Lib/DefaultVertexInput.hlsl"

[[vk::binding(0, 1)]] ConstantBuffer<LightBuffer> lightingBuffer;
[[vk::push_constant]]ConstantBuffer<UniformBufferObject> ubo;

static const float4x4 biasMat = float4x4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0 );

[shader("vertex")]
VSOut main(VertexInput input) {
    VSOut output = (VSOut)0.0;
    output.PositionCS = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.Position, 1.0))));
    output.PosWS = mul(ubo.model , float4(input.Position, 1));
    output.Color = float4(input.Color, 1.0);
    output.TexCoord= float4(input.TexCoord, 0, 0);
    output.Normal = float4(input.Normal,0.);
    output.PosOS = float4(input.Position,0.);
    return output;
}

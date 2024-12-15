#include "Lib/DefaultVertexInput.hlsl"
#include "Lib/Lighting.hlsl"

[[vk::binding(0, 1)]]
ConstantBuffer<LightBuffer> lightingBuffer;

[[vk::push_constant]]
ConstantBuffer<UniformBufferObject> ubo;

//Column major
static const float4x4 biasMat = float4x4(
    { 0.5, 0, 0, 0.5 },
    { 0, 0.5, 0, 0.5 },
    { 0, 0, 1.0, 0 },
    { 0, 0, 0, 1.0 });

[shader("vertex")]
VSOut main(VertexInput input)
{
    VSOut output = (VSOut)0.0;
    output.PositionCS = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.Position, 1.0))));
    output.PosWS = mul(ubo.model, float4(input.Position, 1));
    output.Color = float4(input.Color, 1.0);
    output.TexCoord = float4(input.TexCoord, 0, 0);
    output.Normal = float4(input.Normal, 0.);
    output.PosOS = float4(input.Position, 0.);
    output.ShadowCoords = mul(biasMat, mul(lightingBuffer.shadowMatrix, mul(lightingBuffer.lightMatrix, mul(ubo.model, float4(input.Position, 1.0)))));
    //  output.ShadowCoords = mul(float4(input.Position, 1.0), mul(ubo.model, mul(lightingBuffer.lightMatrix, mul(lightingBuffer.shadowMatrix, biasMat))));

    return output;
}

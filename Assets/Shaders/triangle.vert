struct VSInput
{
[[vk::location(0)]] float3 Position : POSITION0;
[[vk::location(1)]] float3 Color : COLOR0;
[[vk::location(2)]] float2 uv : TEXCOORD0;
};

struct UBO
{
	float4x4 modelMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
};

ConstantBuffer<UBO> ubo: register(b0);

struct VSOutput
{
	float4 Pos : SV_POSITION;
    float3 Color : COLOR0;
    float2 uv:TEXCOORD0;
};

VSOutput main(VSInput input, uint VertexIndex : SV_VertexID)
{
	VSOutput output = (VSOutput)0;
	output.Color = input.Color * float(VertexIndex);
    output.uv = input.uv;
	float4x4 modelMatrix =ubo.modelMatrix; 
	float4x4 viewMatrix = ubo.viewMatrix;
	output.Pos = mul(ubo.projectionMatrix, mul(viewMatrix, mul(modelMatrix, float4(input.Position.xyz, 1.0))));
	return output;
}
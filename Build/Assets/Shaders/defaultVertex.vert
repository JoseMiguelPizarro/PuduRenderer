[[vk::binding(32, 0)]]Sampler2D global_textures[];


struct LightBuffer {
    float4 lightDirection;
    float4x4 lightMatrix;
    float4x4 shadowMatrix;
};

[[vk::binding(0, 1)]] ConstantBuffer<LightBuffer> lightingBuffer;

struct UniformBufferObject {
float4x4 model;
float4x4 view;
float4x4 proj;
uint materialId;
};

[[vk::push_constant]]ConstantBuffer<UniformBufferObject> ubo;

struct VertexInput
{
    float3 Position:POSITION;
    float3 Color: COLOR;
    float2 TexCoord: TEXCOORD;
    float3 Normal: NORMAL;
};

struct VSOut
{
float4 PositionCS: SV_POSITION;
float4 Color: COLOR;
float4 TexCoord:TEXCOORD;
float4 Normal:NORMAL;
float4 ShadowCoords:TEXCOORD1;
};

const float4x4 biasMat = float4x4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0 );

[shader("vertex")]
VSOut main(VertexInput input) {
    VSOut output = (VSOut)0.0;
    output.PositionCS = ubo.proj * ubo.view * ubo.model * vec4(input.Position, 1.0);
    output.Color = float4(input.Color, 1.0);
    output.TexCoord= float4(input.TexCoord, 0, 0);
    output.Normal = (ubo.model * float4(input.Normal, 0));
    
    output.ShadowCoords = biasMat * lightingBuffer.shadowMatrix * lightingBuffer.lightMatrix * ubo.model * float4(input.Position, 1.0);
        
    return output;
}

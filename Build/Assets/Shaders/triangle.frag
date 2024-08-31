// Bindless support
[[vk::binding(32, 0)]]Sampler2D global_textures[];

struct LightBuffer {
float4 lightDirection;
float4x4 lightMatrix;
float4x4 shadowMatrix;
};

[[vk::binding(0, 1)]]ConstantBuffer<LightBuffer> lightingBuffer;

struct UniformBufferObject {
float4x4 model;
float4x4 view;
float4x4 proj;
uint materialId;
};

[[vk::push_constant]]ConstantBuffer<UniformBufferObject> ubo;
        
struct VSOut
{
        float4 PositionCS:SV_POSITION;
        float4 Color: COLOR;
        float4 TexCoord:TEXCOORD;
        float4 Normal:NORMAL;
        float4 ShadowCoords:TEXCOORD1;        
};
        
float linearDepth(float d, float near, float far)
{
    return near * far / (far + d * (near - far));
}

vec4 GetLighting(vec3 normal, vec3 lightDirection)
{
    float ndl = dot(normal, lightDirection) * 0.5 + 0.5;
    //    return vec4(ndl);

    return mix(vec4(1), vec4(0.54, 0.95, 1, 0), 1 - ndl);
}
        

////Percentage-Closer Filtering
//float PCF(vec4 shadowCoords)
//{
//    const int samples = 12;
//}

float textureProj(float4 shadowCoord, float2 off)
{
    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
    {
        Sampler2D tex = global_textures[NonUniformResourceIndex(4)];
        float dist = tex.Sample(shadowCoord.xy + off).x;
        if (shadowCoord.w > 0.0 && dist < shadowCoord.z)
        {
            shadow = 0.0f;
        }
    }
    return shadow;
}

[shader("pixel")]
float4 main(VSOut input):SV_TARGET {
    uint id = ubo.materialId;
    Sampler2D tex = global_textures[NonUniformResourceIndex(id)];
    float4 base_colour = tex.Sample(input.TexCoord.xy);

    //Perspective Devide shadow to map 0-1
    float shadow =clamp( textureProj(input.ShadowCoords / input.ShadowCoords.w, float2(0.0)) + 0.5,0,1);
    float4 col =pow(base_colour, float4(1.0 / 2.2)) * GetLighting(normalize(input.Normal.xyz), lightingBuffer.lightDirection.xyz);
    col = lerp(col,float4(0.2,0.2,0.3,1.0),1 - shadow);
    
    return col;
}

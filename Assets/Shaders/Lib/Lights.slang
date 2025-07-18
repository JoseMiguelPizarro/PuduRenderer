#ifndef PUDU_LIGHTS
#define PUDU_LIGHTS

public struct DirectionalLight
{
    public float4 lightDirection;
    public float4 lightColor;
//Shadow view matrix
    public float4x4 lightMatrix;
//Shadow projection matrix
    public float4x4 shadowMatrix;
};

public struct PointLight
{
    public float3 lightPosition;
    public float4 lightColor;
    public float lightRadius;
    public float lightIntensity;
};

public struct LightingBuffer
{
    public DirectionalLight directionalLight;
    public int lightCount;
    public PointLight[64] pointLight;
};

public struct SceneLighting
{
    public ConstantBuffer<LightingBuffer> lights;
    public Sampler2DShadow shadowMap;
    public SamplerCube IBL_Specular; //Convoluted image based lighting env map
    public SamplerCube IBL_Diffuse;
    public Sampler2D BRDF_LUT;
    public uint IBL_Levels;
};
#endif
#if !defined(PUDU_LIGHTING)
#define PUDU_LIGHTING
public struct LightBuffer
{
    public float4 lightDirection;
    public float4x4 lightMatrix;
    public float4x4 shadowMatrix;
};

public struct UniformBufferObject
{
    public float4x4 model;
    public float4x4 view;
    public float4x4 proj;
    public uint materialId;
    public float time;
};

#endif 
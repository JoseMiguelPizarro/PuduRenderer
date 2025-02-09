#if !defined(PUDU_LIGHTING)
#define PUDU_LIGHTING
public struct LightBuffer
{
    public float4 lightDirection;
//Shadow view matrix
    public float4x4 lightMatrix;
//Shadow projection matrix
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
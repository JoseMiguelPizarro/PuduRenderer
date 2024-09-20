#if !defined(PUDU_LIGHTING)
#define PUDU_LIGHTING
struct LightBuffer {
    float4 lightDirection;
    float4x4 lightMatrix;
    float4x4 shadowMatrix;
};

struct UniformBufferObject {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
    uint materialId;
};

#endif
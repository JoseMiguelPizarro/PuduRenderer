#include "Lib/Lighting.hlsl"

module PuduGraphics;

[__AttributeUsage(_AttributeTargets.Var)]
struct BindlessAttribute
{
};

[__AttributeUsage(_AttributeTargets.Var)]
struct ScopeAttribute
{
    string value;
};

public struct GlobalsConstants
{
    public float2 screenSize;
    public float time;
    public float nearPlane;
    public float farPlane;
    public float3 cameraPosWS;
    public float4x4 view;
    public float4x4 proj;
}

public struct Globals
{
    float myFloatValue;
    public Sampler2D shadowMap;
    public Sampler2D normalBuffer;
    public Sampler2D depthBuffer;
    public Sampler2D colorBuffer;
    public ConstantBuffer<LightBuffer> lightingBuffer;
    public ConstantBuffer<GlobalsConstants> constants;
}

// public struct GlobalsBindless
// {
//     public Sampler2D textures[];
// }

[Scope("Global")]
public ParameterBlock<Globals> GLOBALS;

// [Bindless, Scope("Global")]
// public ParameterBlock<GlobalsBindless> GLOBALS_BINDLESS;

[vk::push_constant]
public UniformBufferObject UBO;

#define PERSHADERBIND 2
#define BIND(id) [[vk::binding(id, PERSHADERBIND)]]
#define CONSTANTS(type)   \
    [[vk::push_constant]] \
    ConstantBuffer<type>

// public Sampler2D GetGlobalTexture(uint id)
// {
//     return GLOBALS_BINDLESS.textures[NonUniformResourceIndex(id)];
// }

//This should only be called during the fragment shader. in fragment shader SV_Position is effectively the screen pixel coordinates
public float2 GetPixelScreenPosition(float4 PosCS)
{
    return float2(PosCS.x/GLOBALS.constants.screenSize.x,PosCS.y/GLOBALS.constants.screenSize.y);
};

public float LinearizeDepth(float d)
{
    return LinearizeDepth(d, GLOBALS.constants.nearPlane, GLOBALS.constants.farPlane);
};

public float LinearizeDepth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

public float3 DecodeNormal(float2 n)
{
    n = n*2. - 1.;
    var z = sqrt(1. - n.x*n.x - n.y*n.y);

    return normalize(float3(n.x,n.y,z));
}

public float3 TangentToWorld(float3 v, float3 t, float3 bt, float3 n)
{
    return v.x*t + v.y*n + v.z*bt;
}

public float4x4 GetViewMatrix(){
    return GLOBALS.constants.view;
    }

public float4x4 GetProjectionMatrix()
{
    return GLOBALS.constants.proj;
}
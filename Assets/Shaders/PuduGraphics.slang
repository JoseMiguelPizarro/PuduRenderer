#include "Lib/Lighting.hlsl"

module PuduGraphics;

[__AttributeUsage(_AttributeTargets.Var)]
struct BindlessAttribute
{
    bool value;
};


public struct GlobalsConstants
{
    public float2 screenSize;
}

public struct Globals
{
    public Sampler2D shadowMap;
    public Sampler2D normalBuffer;
    public Sampler2D depthBuffer;
    public ConstantBuffer<LightBuffer> lightingBuffer;
    public ConstantBuffer<GlobalsConstants> constants;
}

public struct GlobalsBindless
{
    public Sampler2D textures[];
}

public ParameterBlock<Globals> GLOBALS;

[Bindless(true)]
public ParameterBlock<GlobalsBindless> GLOBALS_BINDLESS;

[vk::push_constant]
public ConstantBuffer<UniformBufferObject> UBO;

#define PERSHADERBIND 2
#define BIND(id) [[vk::binding(id, PERSHADERBIND)]]
#define CONSTANTS(type)   \
    [[vk::push_constant]] \
    ConstantBuffer<type>

public Sampler2D GetGlobalTexture(uint id)
{
    return GLOBALS_BINDLESS.textures[NonUniformResourceIndex(id)];
}

public float2 GetPixelScreenPosition(float4 PosCS)
{
    return float2(PosCS.x/GLOBALS.constants.screenSize.x,PosCS.y/GLOBALS.constants.screenSize.y);
};

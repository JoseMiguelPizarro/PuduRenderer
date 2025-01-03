#include "Lib/Lighting.hlsl"

module PuduGraphics;

public struct Globals
{
    public Sampler2D shadowMap;
    public ConstantBuffer<LightBuffer> lightingBuffer;
}

public struct GlobalsBindless
{
    public Sampler2D textures[];
}

public ParameterBlock<Globals> GLOBALS;
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

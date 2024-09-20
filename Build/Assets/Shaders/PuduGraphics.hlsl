#include "Lib/Lighting.hlsl"

#if !defined(PUDU_GRAPHICS)
#define PUDU_GRAPHICS
[[vk::binding(32, 0)]]Sampler2D global_textures[];
[[vk:: binding(0, 1)]]ConstantBuffer < LightBuffer > lightingBuffer;

#define BIND(id) [[vk::binding(id,2)]]
#define CONSTANTS(type) [[vk:: push_constant]] ConstantBuffer<type>

#define GET_GLOBAL_TEXTURE(id) global_textures[NonUniformResourceIndex(id)]
#endif
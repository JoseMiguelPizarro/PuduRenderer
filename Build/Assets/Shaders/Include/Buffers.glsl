// Bindless support
// Enable non uniform qualifier extension
#extension GL_EXT_nonuniform_qualifier: enable
layout (set = 0, binding = 32) uniform sampler2D global_textures[];

//layout (set= 1, binding = 0) uniform LightBuffer{
//    vec4 lightDirection;
//    mat4 dirLightMatrix;
//    mat4 shadowMatrix;
//}lightBuffer;
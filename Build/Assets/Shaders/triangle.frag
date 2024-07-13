#version 450

// Bindless support
// Enable non uniform qualifier extension

#extension GL_EXT_nonuniform_qualifier: enable
layout(binding=0) uniform sampler2D shadowMap;

layout (set = 0, binding = 32) uniform sampler2D global_textures[];

layout(set = 1, binding = 0) uniform LightBuffer{
    vec4 lightDirection;
    mat4 lightMatrix;
    mat4 shadowMatrix;
} lightingBuffer ;


layout (push_constant, std430) uniform Material {
    layout (offset = 192) uint materialId;
};

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec4 outColor;

vec4 GetLighting(vec3 normal, vec3 lightDirection)
{
    float ndl = dot(normal, lightDirection) * 0.5 + 0.5;
    //    return vec4(ndl);

    return mix(vec4(1), vec4(0.54, 0.95, 1, 0), 1 - ndl);
}

void main() {
    uint id = materialId;
    vec4 base_colour = texture(global_textures[nonuniformEXT(id)], inTexCoord);
    outColor = pow(base_colour, vec4(1.0 / 2.2)) * GetLighting(normalize(inNormal), lightingBuffer.lightDirection.xyz);
    
    vec2 screenUV = gl_FragCoord.xy/1048;
    outColor = texture(global_textures[nonuniformEXT(4)],screenUV)/2.0;
}

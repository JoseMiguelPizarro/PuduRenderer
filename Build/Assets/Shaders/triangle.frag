#version 450

// Bindless support
// Enable non uniform qualifier extension

#extension GL_EXT_nonuniform_qualifier: enable
layout (set = 0, binding = 32) uniform sampler2D global_textures[];

layout(set = 1, binding = 0) uniform LightBuffer{
    vec4 lightDirection;
    mat4 lightMatrix;
    mat4 shadowMatrix;
} lightingBuffer ;

layout (push_constant, std430) uniform Material {
    layout (offset = 192) uint materialId;
};

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec4 inTexCoord;
layout (location = 2) in vec4 inNormal;
layout (location = 3) in vec4 inShadowCoords;

layout (location = 0) out vec4 outColor;

vec4 GetLighting(vec3 normal, vec3 lightDirection)
{
    float ndl = dot(normal, lightDirection) * 0.5 + 0.5;
    //    return vec4(ndl);

    return mix(vec4(1), vec4(0.54, 0.95, 1, 0), 1 - ndl);
}

void main() {
    uint id = materialId;
    vec4 base_colour = texture(global_textures[nonuniformEXT(id)], inTexCoord.xy);
    outColor = pow(base_colour, vec4(1.0 / 2.2)) * GetLighting(normalize(inNormal.xyz), lightingBuffer.lightDirection.xyz);

    vec2 screenUV = gl_FragCoord.xy/1048;
    float d = texture(global_textures[nonuniformEXT(4)],screenUV).x/10.0;
    outColor = vec4(d,d,d,1);
}

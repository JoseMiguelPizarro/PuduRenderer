#version 450

// Bindless support
// Enable non uniform qualifier extension

#extension GL_EXT_nonuniform_qualifier: enable
layout (set = 0, binding = 32) uniform sampler2D global_textures[];

layout (set = 1, binding = 0) uniform LightBuffer {
    vec4 lightDirection;
    mat4 lightMatrix;
    mat4 shadowMatrix;
} lightingBuffer;

layout (push_constant, std430) uniform Material {
    layout (offset = 192) uint materialId;
};

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec4 inTexCoord;
layout (location = 2) in vec4 inNormal;
layout (location = 3) in vec4 inShadowCoords;

layout (location = 0) out vec4 outColor;

float linearDepth(float d, float near, float far)
{
    return near * far / (far + d * (near - far));
}

vec4 GetLighting(vec3 normal, vec3 lightDirection)
{
    float ndl = dot(normal, lightDirection) * 0.5 + 0.5;
    //    return vec4(ndl);

    return mix(vec4(1), vec4(0.54, 0.95, 1, 0), 1 - ndl);
}

float textureProj(vec4 shadowCoord, vec2 off)
{
    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
    {
        float dist = texture(global_textures[nonuniformEXT(4)], shadowCoord.st + off).r;
        if (shadowCoord.w > 0.0 && dist < shadowCoord.z)
        {
            shadow = 0.0f;
        }
    }
    return shadow;
}

void main() {
    uint id = materialId;
    vec4 base_colour = texture(global_textures[nonuniformEXT(id)], inTexCoord.xy);

    float shadow =clamp( textureProj(inShadowCoords / inShadowCoords.w, vec2(0.0)) + 0.5,0,1);
    vec4 col =pow(base_colour, vec4(1.0 / 2.2)) * GetLighting(normalize(inNormal.xyz), lightingBuffer.lightDirection.xyz);
    col = mix(col,vec4(0.2,0.2,0.3,1.0),1 - shadow);
    outColor = col;
}

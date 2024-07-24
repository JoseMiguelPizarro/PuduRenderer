#version 450

layout (push_constant, std430) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

#extension GL_EXT_nonuniform_qualifier: enable
layout (set = 0, binding = 32) uniform sampler2D global_textures[];

layout(set = 1, binding = 0) uniform LightBuffer{
    vec4 lightDirection;
    mat4 lightMatrix;
    mat4 shadowMatrix;
} lightingBuffer ;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fragTexCoord;
layout (location = 2) out vec4 normal;
layout (location = 3) out vec4 shadowCoords;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = vec4(inColor,1.0);
    fragTexCoord = vec4(inTexCoord,0,0);
    normal = (ubo.model * vec4(inNormal, 0));
    shadowCoords = lightingBuffer.shadowMatrix * ubo.model * vec4(inPosition,1.0);
}

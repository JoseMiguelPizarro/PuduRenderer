#version 450

// Bindless support
// Enable non uniform qualifier extension
#extension GL_EXT_nonuniform_qualifier: enable
layout (set = 0, binding = 32) uniform sampler2D global_textures[];

layout (push_constant, std430) uniform Material {
    layout (offset = 192) uint materialId;
};

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;

void main() {
    uint id = materialId;
    vec4 base_colour = texture(global_textures[nonuniformEXT(id)], inTexCoord);
    outColor = pow(base_colour, vec4(1.0 / 2.2));
    outColor = vec4(1,0,0,1);
}

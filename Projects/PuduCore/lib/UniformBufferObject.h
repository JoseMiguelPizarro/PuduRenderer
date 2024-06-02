#pragma once
#include "glm/glm.hpp"
using namespace glm;

struct UniformBufferObject {
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 ProjectionMatrix;
};

//16bit alignment 
struct  RenderConstants {
	alignas(16) uint32_t materialId;
	alignas(16) vec3 g_LightDirection;
};
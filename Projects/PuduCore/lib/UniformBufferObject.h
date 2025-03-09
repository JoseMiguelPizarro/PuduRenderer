#pragma once
#include "glm/glm.hpp"
using namespace glm;

struct UniformBufferObject {
	mat4 modelMatrix;
	uint32_t materialId;
	float time;
};

//16bit alignment 
struct  RenderConstants {
	alignas(16) uint32_t materialId;
};
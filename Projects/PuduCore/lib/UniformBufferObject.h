#pragma once
#include "glm/glm.hpp"
using namespace glm;

struct UniformBufferObject {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 ProjectionMatrix;
};
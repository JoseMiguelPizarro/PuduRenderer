#pragma once
#include "glm/glm.hpp"
#include "PuduCore.h"
using namespace glm;

namespace Pudu
{
    struct UniformBufferObject
    {
        mat4 modelMatrix;
        uint4 custom;
        u32 materialId;
        float time;
    };
}

#pragma once
#include "glm/glm.hpp"
#include "PuduCore.h"
using namespace glm;

namespace Pudu
{
    struct UniformBufferObject
    {
        mat4 modelMatrix;
        u32 materialId;
        float time;
        uint4 custom;
    };

    //16bit alignment
    struct RenderConstants
    {
        alignas(16) uint32_t materialId;
    };
}

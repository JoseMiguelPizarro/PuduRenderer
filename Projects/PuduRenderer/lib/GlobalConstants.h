#pragma once
#include "glm/glm.hpp"
#include "PuduCore.h"

namespace Pudu
{
    struct GlobalConstants
    {
        alignas(16) mat4 viewMatrix;
        alignas(16) mat4 projectionMatrix;
        alignas(16) mat4 viewProjectionMatrix;
        float4 cameraPosWS;
        float4 nearPlane;
        float4 farPlane;
        float4 screenSize;
        float4 time;
    };
}

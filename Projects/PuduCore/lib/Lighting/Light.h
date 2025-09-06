#pragma once
#include "PuduCore.h"
#include <glm/common.hpp>
#include "Projection.h"

namespace Pudu
{
    enum class LightType
    {
        Directional,
        Spotlight,
        Point,
    };
    class Light
    {
    public:
        float range;
        float intensity;
        LightType type;

        float4 color = float4(1.0f);

        Projection Projection;

        float GetIlluminance();
    };
}

#pragma once
#include "PuduCore.h"
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include "../Entities/Entity.h"
#include "Projection.h"

namespace Pudu
{
    class Light : public Entity
    {
    public:
        float radius;
        float intensity;

        float4 color = float4(1.0f);
        float4x4 GetShadowMatrix();
        float4x4 GetLightMatrix();
        float3 Direction();
        Projection Projection;

        float GetIlluminance();
    };
}

#pragma once
#include "Lighting/Light.h"
#include <glm/fwd.hpp>
#include "PuduCore.h"

namespace Pudu
{
    struct DirectionalLightData
    {
        float4 lightDirection;
        float4 lightColor;

        //Shadow view matrix
        float4x4 lightMatrix;
        //Shadow projection matrix
        float4x4 shadowMatrix;
    };

    struct PointLightData
    {
        float3 lightPosition;
        float4 lightColor;
        float lightRadius;
        float lightIntensity;
    };

    struct LightBuffer
    {
        DirectionalLightData directionalLight;
        int lightCount;
        PointLightData pointLight[64];
    };
}

//
// Created by Hojaverde on 9/5/2025.
//

#pragma once
#include "Entities/Entity.h"
#include "Lighting/Light.h"

namespace Pudu
{
    class LightEntity : public Entity
    {
    public:
        Light& GetLightData();
        void SetLightData(const Light& light);
        float4x4 GetShadowMatrix();
        float4x4 GetLightMatrix();
        float3 Direction();
        float4 GetColor();
        Projection GetProjection();
        void SetProjection(const Projection& projection);

    private:
        Light m_light;
    };
}

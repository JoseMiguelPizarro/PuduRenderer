//
// Created by Hojaverde on 9/5/2025.
//

#include "../../lib/Entities/LightEntity.h"
#include "PuduMath.h"

namespace Pudu {
    Light& LightEntity::GetLightData()
    {
        return m_light;
    }

    void LightEntity::SetLightData(const Light& light)
    {
        m_light = light;
    }

    float4x4 LightEntity::GetShadowMatrix()
    {
        return m_light.Projection.GetProjectionMatrix();
    }

    float4x4 LightEntity::GetLightMatrix()
    {
        vec3 forward = m_transform.GetForward();
        return LookAtInverse(m_transform.GetLocalPosition(), forward, {0, 1, 0});
    }

    vec3 LightEntity::Direction()
    {
        return { m_transform.GetForward()};
    }

    float4 LightEntity::GetColor()
    {
        return m_light.color;
    }

    Projection LightEntity::GetProjection()
    {
        return m_light.Projection;
    }

    LightType LightEntity::GetLightType()
    {
        return m_light.type;
    }

    void LightEntity::SetProjection(const Projection& projection)
    {
        m_light.Projection = projection;
    }
} // Pudu
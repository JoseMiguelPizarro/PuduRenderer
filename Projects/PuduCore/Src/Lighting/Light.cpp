#include <glm/fwd.hpp>
#include "Lighting/Light.h"
#include <PuduMath.h>

namespace Pudu
{
	glm::mat4 Light::GetShadowMatrix()
	{
		return Projection.GetProjectionMatrix();
	}

	glm::mat4 Light::GetLightMatrix()
	{
		vec3 forward = m_transform.GetForward();
		return Pudu::LookAtInverse(m_transform.GetLocalPosition(), forward, {0, 1, 0});
	}

	glm::vec3 Light::Direction() 
	{
		return { m_transform.GetForward()};
	}

	float Light::GetIlluminance()
	{
		return intensity / (radius * radius);
	}
}


#include <glm/fwd.hpp>
#include "Lighting/Light.h"

glm::mat4 Pudu::Light::GetShadowMatrix()
{
		return glm::mat4();
}

glm::mat4 Pudu::Light::GetLightMatrix()
{

	return glm::mat4();
}

float Pudu::Light::GetIlluminance()
{
	return intensity / (radius * radius);
}

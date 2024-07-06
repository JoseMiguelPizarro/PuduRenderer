#pragma once
#include "Lighting/Light.h"
#include <glm/fwd.hpp>

namespace Pudu
{
	struct LightBuffer
	{
		alignas(16)	glm::vec3 lightDirection;
		glm::mat4 dirLightMatrix;
		glm::mat4 shadowMatrix;
	};
}



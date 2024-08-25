#pragma once
#include "Lighting/Light.h"
#include <glm/fwd.hpp>

namespace Pudu
{
	struct LightBuffer
	{
		glm::vec4 lightDirection;
		glm::mat4 dirLightMatrix;
		glm::mat4 shadowMatrix;
	};
}



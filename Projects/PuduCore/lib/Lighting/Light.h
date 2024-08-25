#pragma once
#include "PuduCore.h"
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include "Entity.h"
#include "Projection.h"

namespace Pudu
{
	class Light :public Entity
	{
	public:
		float radius;
		float intensity;

		glm::mat4 GetShadowMatrix();
		glm::mat4 GetLightMatrix();
		glm::vec3 Direction();
		Projection Projection;
	

		float GetIlluminance();
	};
}


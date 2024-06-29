#pragma once
#include "PuduCore.h"
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include "Entity.h"

namespace Pudu
{
	class Light :Entity
	{
	public:
		glm::vec3 direction;
	};
}


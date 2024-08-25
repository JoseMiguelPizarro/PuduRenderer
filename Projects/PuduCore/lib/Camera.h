#pragma once
#include "Transform.h"
#include <Projection.h>

using namespace glm;

namespace Pudu {
	class Camera
	{
	public:
		Transform Transform;
		Projection Projection;

		mat4 GetViewMatrix();
		mat4 GetInverseViewMatrix();
	};
}
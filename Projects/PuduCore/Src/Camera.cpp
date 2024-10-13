#include "Camera.h"

#include "glm/glm.hpp"
#include "PuduMath.h"

using namespace glm;

namespace Pudu {
	mat4 Camera::GetViewMatrix()
	{
		vec3 forward = Transform.GetForward();
		return Pudu::LookAtInverse(Transform.GetLocalPosition(), forward, {0, 1, 0});
	}

	mat4 Camera::GetInverseViewMatrix()
	{
		return Transform.GetTransformationMatrix();
	}
}
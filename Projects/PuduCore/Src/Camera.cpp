#include "Camera.h"

#include "glm/glm.hpp"
#include "PuduMath.h"

using namespace glm;

namespace Pudu {
	mat4 Camera::GetViewMatrix()
	{
		vec3 forward = Transform.GetForward();
		return Pudu::LookAtInverse(Transform.LocalPosition, forward, { 0, 1, 0 });
	}

	mat4 Camera::GetInverseViewMatrix()
	{
		return Transform.GetTransformationMatrix();
	}

	mat4 Camera::GetPerspectiveMatrix()
	{
		return Pudu::PerspectiveMatrix(
			Fov, Height / Width, 0.1f, 1000.0f);
	}
}
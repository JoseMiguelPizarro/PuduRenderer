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

	mat4 Camera::GetProjectionMatrix()
	{
		if (projectionType == Perspective)
		{
			return Pudu::PerspectiveMatrix(
				Fov, Width / Height, 0.1f, 1000.0f);
		}
		else {

			float halfWidth = Width * 0.5f;
			float halfHeight = Height * 0.5f;

			return Pudu::OrthograpicMatrix(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
		}
	}
}
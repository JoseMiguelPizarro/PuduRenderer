#include "Projection.h"
#include "glm/matrix.hpp"
#include <PuduMath.h>


namespace Pudu
{
	glm::mat4 Projection::GetProjectionMatrix()
	{
		if (projectionType == Perspective)
		{
			return Pudu::PerspectiveMatrix(
				Fov, Width / Height, nearPlane, farPlane);
		}
		else {

			float halfWidth = Width * 0.5f;
			float halfHeight = Height * 0.5f;

			return Pudu::OrthograpicMatrix(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
		}
	}
}

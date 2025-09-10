#include <array>

#include "Projection.h"
#include "Math/PuduMath.h"


namespace Pudu
{
	void Projection::GetFrustumPlanes(std::array<Plane, 6> planes)
	{
		
	}

	mat4 Projection::GetProjectionMatrix()
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

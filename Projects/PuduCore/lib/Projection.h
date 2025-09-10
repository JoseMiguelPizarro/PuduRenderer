#pragma once
#include "Math/Plane.h"

namespace Pudu
{
	struct Projection
	{
		enum ProjectionType {
			Perspective,
			Ortographic
		};

		float Width = 5;
		float Height = 5;
		float Fov = 45;
		float nearPlane = 1;
		float farPlane = 100;
		ProjectionType projectionType = Perspective;


		void GetFrustumPlanes(std::array<Plane,6> planes);

		glm::mat4 GetProjectionMatrix();
	};
}

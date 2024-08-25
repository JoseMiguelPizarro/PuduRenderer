#pragma once
#include "glm/matrix.hpp"

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

		glm::mat4 GetProjectionMatrix();
	};
}
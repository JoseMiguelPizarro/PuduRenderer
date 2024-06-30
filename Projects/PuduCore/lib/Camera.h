#pragma once
#include "Transform.h"

using namespace glm;

namespace Pudu {
	class Camera
	{


	public:
		enum ProjectionType {
			Perspective,
			Ortographic
		};


		Transform Transform;
		float Width = 5;
		float Height = 5;
		float Fov = 45;
		float nearPlane = 0;
		float farPlane = 100;
		ProjectionType projectionType = Perspective;

		mat4 GetViewMatrix();
		mat4 GetInverseViewMatrix();
		mat4 GetPerspectiveMatrix();
	};
}
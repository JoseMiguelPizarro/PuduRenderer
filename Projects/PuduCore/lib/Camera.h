#pragma once
#include "Transform.h"
#include <Projection.h>
#include <Color.h>

using namespace glm;

namespace Pudu {
	class Camera
	{
	public:
		Transform Transform;
		Projection Projection;

		mat4 GetViewMatrix();
		mat4 GetInverseViewMatrix();
		void SetClearColor(Color color);
		Color GetClearColor() const;

	private:
		Color m_clearColor;
	};
}
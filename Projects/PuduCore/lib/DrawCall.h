#pragma once
#include <glm/fwd.hpp>
#include "PuduCore.h"
#include "Model.h"

namespace Pudu
{
	struct DrawCall
	{
		Model ModelPtr;
		SPtr<Mesh> MeshPtr;
		Material MaterialPtr;
		mat4 TransformMatrix;

		DrawCall() {};
		DrawCall(Model  model, SPtr<Mesh> mesh, Material material)
		{
			ModelPtr = model;
			MeshPtr = mesh;
			MaterialPtr = material;
		}
	};
}

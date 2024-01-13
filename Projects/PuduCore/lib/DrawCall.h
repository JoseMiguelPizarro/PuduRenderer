#pragma once

#include "Model.h"
namespace Pudu
{
	class DrawCall
	{
	public:
		Model* ModelPtr;
		Mesh* MeshPtr;
		Material* MaterialPtr;

		DrawCall() {}
		DrawCall(Model* model, Mesh* mesh, Material* material)
		{
			ModelPtr = model;
			MeshPtr = mesh;
			MaterialPtr = material;
		}
	};
}

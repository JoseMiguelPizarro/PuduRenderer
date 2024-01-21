#pragma once

#include "PuduCore.h"
#include "Model.h"
namespace Pudu
{
	class DrawCall
	{
	public:
		Model ModelPtr;
		SPtr<Mesh> MeshPtr;
		Material MaterialPtr;
		mat4 TransformMatrix;

		DrawCall() {}
		DrawCall(Model  model, SPtr<Mesh> mesh, Material material)
		{
			ModelPtr = model;
			MeshPtr = mesh;
			MaterialPtr = material;
		}
	};
}

#pragma once
#include <glm/fwd.hpp>
#include "PuduCore.h"
#include "Model.h"

namespace Pudu
{
	struct DrawCall
	{
		Model* ModelPtr;
		SPtr<Mesh> MeshPtr;
		SPtr<Material> MaterialPtr;
		mat4 TransformMatrix;


		SPtr<Material> GetRenderMaterial();
		void SetReplacementMaterial(SPtr<Material> material);
		bool HasReplacementMaterial();

		DrawCall() {};
		DrawCall(Model*  model, SPtr<Mesh> mesh, SPtr<Material> material)
		{
			ModelPtr = model;
			MeshPtr = mesh;
			MaterialPtr = material;
		}

	private:
		SPtr<Material> m_replacementMaterial;
	};
}

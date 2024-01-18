#include "Scene.h"

void Pudu::Scene::AddModel(Model& model)
{
	for (size_t i = 0; i < model.Meshes.size(); i++)
	{
		Material material = {};
		if (i >= model.Materials.size())
		{
			material = model.Materials[0];
		}

		Model m = model;
		DrawCall dc(m, model.Meshes[i], material);
		m_DrawCalls.push_back(dc);
	}
}

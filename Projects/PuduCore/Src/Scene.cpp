#include "Scene.h"

void Pudu::Scene::AddModel(Model* model)
{
	for (size_t i = 0; i < model->Meshes.size(); i++)
	{
		Material* material = nullptr;
		if (i >= model->Materials.size())
		{
			material = &model->Materials[0];
		}

		DrawCall dc(model, model->Meshes[i], material);
		m_DrawCalls.push_back(dc);
	}
}

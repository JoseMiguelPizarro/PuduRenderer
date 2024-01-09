#include "Scene.h"

void Pudu::Scene::AddModel(Model* model)
{
	DrawCall dc(model);
	m_DrawCalls.push_back(dc);
}

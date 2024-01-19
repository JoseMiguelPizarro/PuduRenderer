#include "Scene.h"

namespace Pudu {
	void Scene::AddEntity(EntitySPtr entity)
	{
		m_entities.push_back(entity);
		entity->AttatchToScene(*this);
	}
	void Scene::AddEntities(std::vector<EntitySPtr> entities)
	{
		for (auto e : entities)
		{
			AddEntity(e);
		}
	}
	void Scene::RemoveEntity(EntitySPtr entity)
	{
		//TODO
	}

	void Scene::AddRendererEntity(RenderEntitySPtr renderEntity)
	{
		m_renderEntities.push_back(renderEntity);

		auto& model = renderEntity->GetModel();
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
	void Scene::RemoveRenderEntity(RenderEntitySPtr renderEntity)
	{
		//TODO
	}
	std::vector<EntitySPtr> Scene::GetEntities()
	{
		return m_entities;
	}
	std::vector<RenderEntitySPtr> Scene::GetRenderEntities()
	{
		return m_renderEntities;
	}
}

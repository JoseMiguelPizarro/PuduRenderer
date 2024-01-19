#include "EntityManager.h"

namespace Pudu {

	std::vector<RenderEntitySPtr> EntityManager::m_renderEntities;
	std::vector<EntitySPtr> EntityManager::m_entities;

	EntitySPtr EntityManager::AllocateEntity()
	{
		auto e = std::make_shared<Entity>();
		e->m_entitySPtr = e;
		m_entities.push_back(e);


		return e;
	}

	EntitySPtr EntityManager::AllocateEntity(std::string& name)
	{
		auto e = AllocateEntity();
		e->SetName(name);

		return e;
	}

	RenderEntitySPtr EntityManager::AllocateRenderEntity()
	{
		auto e = std::make_shared<RenderEntity>();
		e->m_entitySPtr = e;
		m_renderEntities.push_back(e);

		return e;
	}

	RenderEntitySPtr EntityManager::AllocateRenderEntity(std::string& name)
	{
		auto e = AllocateRenderEntity();
		e->SetName(name);

		return e;
	}

	RenderEntitySPtr EntityManager::AllocateRenderEntity(Model model)
	{
		auto e = AllocateRenderEntity();
		e->SetModel(model);

		return e;
	}

	RenderEntitySPtr EntityManager::AllocateRenderEntity(std::string& name, Model model)
	{
		auto e = AllocateRenderEntity(name);
		e->SetModel(model);

		return e;
	}

	void EntityManager::DestroyEntity(RenderEntitySPtr entity)
	{
	}
}


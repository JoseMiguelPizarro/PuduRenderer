#include "RenderEntityManager.h"

namespace Pudu {

	RenderEntitySPtr RenderEntityManager::AllocateEntity()
	{
		return std::make_shared<RenderEntity>();
	}

	RenderEntitySPtr RenderEntityManager::AllocateEntity(Model model)
	{
		auto entity = AllocateEntity();
		entity->SetModel(model);

		return entity;
	}

	void RenderEntityManager::DestroyEntity(RenderEntitySPtr entity)
	{
	}
}


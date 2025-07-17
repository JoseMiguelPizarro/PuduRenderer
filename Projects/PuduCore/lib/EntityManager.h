#pragma once
#include "EntityFwd.h"
#include "IEntityProvider.h"
#include "RenderEntity.h"


namespace Pudu {

	class EntityManager: public IEntityProvider
	{
	public:
		EntityHandle AllocateEntity();
		EntityHandle AllocateEntity(std::string& name);
		EntitySPtr GetEntity(Handle handle);

		 EntityHandle AllocateRenderEntity();
		 EntityHandle AllocateRenderEntity(const char*  name);
		 EntityHandle AllocateRenderEntity(Model model);
		 EntityHandle AllocateRenderEntity(const char* name, Model model);
		 void DestroyEntity(EntityHandle entity);

		 EntitySPtr GetResource(Handle id) override;
	 private:
		//TODO: IMPLEMENT WITH RESOURCE POOL
		std::vector<EntitySPtr> m_entities;
	};
}


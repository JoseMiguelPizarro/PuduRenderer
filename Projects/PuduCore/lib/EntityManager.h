#pragma once
#include "Entity.h"
#include "RenderEntity.h"


namespace Pudu {

	class EntityManager
	{
	public:

		static EntitySPtr AllocateEntity();
		static EntitySPtr AllocateEntity(std::string& name);

		static RenderEntitySPtr AllocateRenderEntity();
		static RenderEntitySPtr AllocateRenderEntity(std::string& name);
		static RenderEntitySPtr AllocateRenderEntity(Model model);
		static RenderEntitySPtr AllocateRenderEntity(std::string& name, Model model);
		static void DestroyEntity(RenderEntitySPtr entity);

	private:
		static std::vector<RenderEntitySPtr> m_renderEntities;
		static std::vector<EntitySPtr> m_entities;
	};
}


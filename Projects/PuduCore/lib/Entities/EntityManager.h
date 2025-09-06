#pragma once
#include "Entity.h"
#include "Input.h"
#include "RenderEntity.h"


namespace Pudu {

    template<typename T>
    concept EntityDerived = std::is_base_of_v<Entity, T>;

	class EntityManager
	{
	public:

		static EntitySPtr AllocateEntity();
		static EntitySPtr AllocateEntity(std::string& name);

		static RenderEntitySPtr AllocateRenderEntity();
		static RenderEntitySPtr AllocateRenderEntity(const char*  name);
		static RenderEntitySPtr AllocateRenderEntity(Model model);
		static RenderEntitySPtr AllocateRenderEntity(const char* name, Model model);
		static void DestroyEntity(RenderEntitySPtr entity);

		template<EntityDerived T>
		static SPtr<T> AllocateEntity()
		{
			auto e = std::make_shared<T>();
			m_entities.push_back(std::dynamic_pointer_cast<Entity>(e));
			return e;
		}

		template<EntityDerived T>
		static SPtr<T> AllocateEntity(std::string& name)
		{
			auto e = std::make_shared<T>();
			e->SetName(name);
			e->m_entitySPtr = e;
			m_entities.push_back(std::dynamic_pointer_cast<Entity>(e));
			return e;
		}

		template<EntityDerived T>
		static void GetAllEntitiesOfType(std::vector<SPtr<T>>& entities)
		{
			for (auto& e : m_entities)
			{
				if (auto castEntity = std::dynamic_pointer_cast<T>(e))
				{
					entities.push_back(castEntity);
				}
			}
		}

	private:
		static std::vector<RenderEntitySPtr> m_renderEntities;
		static std::vector<EntitySPtr> m_entities;
	};
}


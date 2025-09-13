#pragma once
#include <vector>
#include "PuduTime.h"
#include "Camera.h"
#include "Entities/LightEntity.h"
#include "Entities/Entity.h"
#include "Entities/RenderEntity.h"
#include "Entities/EntityManager.h"

namespace Pudu {

	class Scene
	{
	public:
		Camera* camera = nullptr;

		void AddEntity(const EntitySPtr& entity);
		void AddEntities(std::vector<EntitySPtr> entities);
		void RemoveEntity(EntitySPtr entity);
		void DrawImGui();
		std::vector<EntitySPtr> GetEntities();
		std::vector<RenderEntitySPtr> GetRenderEntities();
		EntitySPtr sceneRoot;
		PuduTime* time;

		Scene() {
			sceneRoot = EntityManager::AllocateEntity();
			sceneRoot->SetName("SceneRoot");
			AddEntity(sceneRoot);
		}
		Scene(PuduTime* time) :Scene() {
			Time = time;
		}

		PuduTime* Time = nullptr;

		template<EntityDerived T>
		void GetEntitiesOfType(std::vector<SPtr<T>>& entities)
		{
			entities.clear();

			for ( auto& entity : m_entities)
			{
				if (auto e = std::dynamic_pointer_cast<T>(entity))
				{
					entities.push_back(e);
				}
			}
		}

	private:
		friend class Entity;
		friend class RenderEntity;

		std::vector<EntitySPtr> m_entities;
		void RemoveEntityInternal(EntitySPtr entity);
	};
}


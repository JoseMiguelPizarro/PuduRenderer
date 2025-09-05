#pragma once
#include <vector>
#include "PuduTime.h"
#include "Camera.h"
#include "Entities/Entity.h"
#include "Entities/RenderEntity.h"
#include "Entities/EntityManager.h"
#include "Lighting/Light.h"

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
		Light* directionalLight;

		Scene() {
			sceneRoot = EntityManager::AllocateEntity();
			sceneRoot->SetName("SceneRoot");
			AddEntity(sceneRoot);
		}
		Scene(PuduTime* time) :Scene() {
			Time = time;
		}

		PuduTime* Time = nullptr;

	private:
		friend class Entity;
		friend class RenderEntity;

		std::vector<EntitySPtr> m_entities;
		void RemoveEntityInternal(EntitySPtr entity);
	};
}


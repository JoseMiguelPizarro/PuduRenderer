#pragma once
#include <vector>
#include "DrawCall.h"
#include "PuduTime.h"
#include "Camera.h"
#include "Entity.h"
#include "RenderEntity.h"
#include "EntityManager.h"

namespace Pudu {
	class Scene
	{
	public:
		void AddEntity(EntitySPtr entity);
		void AddEntities(std::vector<EntitySPtr> entities);
		void RemoveEntity(EntitySPtr entity);
		void RemoveRenderEntity(RenderEntitySPtr renderEntity);
		std::vector<EntitySPtr> GetEntities();
		std::vector<RenderEntitySPtr> GetRenderEntities();
		EntitySPtr SceneRoot;

		std::vector<DrawCall> GetDrawCalls() {
			return m_DrawCalls;
		}

		Camera* Camera = nullptr;

		Scene() {
			SceneRoot = EntityManager::AllocateEntity();
			SceneRoot->SetName("SceneRoot");
			AddEntity(SceneRoot);
		}
		Scene(PuduTime* time) :Scene() {
			Time = time;
		}

		PuduTime* Time = nullptr;

	private:
		friend class Entity;
		friend class RenderEntity;

		void AddRendererEntity(RenderEntitySPtr renderEntity);
		std::vector<DrawCall> m_DrawCalls;
		std::vector<EntitySPtr> m_entities;
		std::vector<RenderEntitySPtr> m_renderEntities;
	};
}


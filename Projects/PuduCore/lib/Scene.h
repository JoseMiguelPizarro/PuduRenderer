#pragma once
#include <vector>
#include "DrawCall.h"
#include "PuduTime.h"
#include "Camera.h"
#include "Entity.h"
#include "RenderEntity.h"
#include "EntityManager.h"
#include "Resources/Resources.h"
#include "Lighting/Light.h"

namespace Pudu {
	class Scene
	{
	public:
		Camera* camera = nullptr;

		void AddEntity(EntitySPtr entity);
		void AddEntities(std::vector<EntitySPtr> entities);
		void RemoveEntity(EntitySPtr entity);
		void RemoveRenderEntity(RenderEntitySPtr renderEntity);
		void DrawImGui();
		std::vector<EntitySPtr> GetEntities();
		std::vector<RenderEntitySPtr> GetRenderEntities();
		EntitySPtr sceneRoot;
		PuduTime* time;
		Light* directionalLight;

		std::vector<DrawCall> GetDrawCalls() {
			return m_DrawCalls;
		}


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

		void AddRendererEntity(RenderEntitySPtr renderEntity);
		std::vector<DrawCall> m_DrawCalls;
		std::vector<EntitySPtr> m_entities;
		std::vector<RenderEntitySPtr> m_renderEntities;
	};
}


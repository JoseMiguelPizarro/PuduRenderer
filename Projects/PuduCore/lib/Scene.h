#pragma once
#include <vector>
#include "DrawCall.h"
#include "PuduTime.h"
#include "Camera.h"
#include "Entity.h"
#include "RenderEntity.h"

namespace Pudu {
	class Scene
	{
	public:
		void AddEntity(EntitySPtr entity);
		void RemoveEntity(EntitySPtr entity);
		void AddRendererEntity(RenderEntitySPtr renderEntity);
		void RemoveRenderEntity(RenderEntitySPtr renderEntity);

		std::vector<DrawCall> GetDrawCalls() {
			return m_DrawCalls;
		}

		Camera* Camera = nullptr;

		Scene() {}
		Scene(PuduTime* time) {
			Time = time;
		}

		PuduTime* Time = nullptr;

	private:
		std::vector<DrawCall> m_DrawCalls;
		std::vector<EntitySPtr> m_entities;
		std::vector<RenderEntitySPtr> m_renderEntities;
	};
}


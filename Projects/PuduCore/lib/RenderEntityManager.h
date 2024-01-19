#pragma once

#include "RenderEntity.h"

namespace Pudu {

	class RenderEntityManager
	{
	public:
		static RenderEntitySPtr AllocateEntity();
		static RenderEntitySPtr AllocateEntity(Model model);
		static void DestroyEntity(RenderEntitySPtr entity);

	private:
		static std::vector<RenderEntitySPtr> m_renderEntities;
	};
}


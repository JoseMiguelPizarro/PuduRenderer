#pragma once
#include <memory>
#include "Entity.h"
#include "Model.h"

namespace Pudu {
	
	class RenderEntity :public Entity
	{
		typedef std::shared_ptr<RenderEntity> RenderEntitySPtr;
	public:
		Model& GetModel();
		void SetModel(Model& model);
		void AttatchToScene(Scene& scene) override;

	private:
		Model m_model;
	};

	typedef std::shared_ptr<RenderEntity> RenderEntitySPtr;
}


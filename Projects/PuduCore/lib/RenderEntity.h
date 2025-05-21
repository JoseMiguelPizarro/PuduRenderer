#pragma once
#include <memory>
#include "Entity.h"
#include "Model.h"
#include "RenderSettings.h"

namespace Pudu {
	
	class RenderEntity :public Entity
	{
		typedef std::shared_ptr<RenderEntity> RenderEntitySPtr;
	public:

		std::string ClassName() override {
			return "RenderEntity";
		}

		Model* GetModel();
		void SetModel(Model& model);
		void AttatchToScene(Scene& scene) override;
		RenderSettings& GetRenderSettings();

	protected:
		Model m_model = {};
		RenderSettings m_renderSettings = {};
	};

	typedef std::shared_ptr<RenderEntity> RenderEntitySPtr;
}


#pragma once
#include "Entity.h"
#include "Model.h"
#include "RenderSettings.h"
#include "EntityFwd.h"

namespace Pudu {
	
	class RenderEntity :public Entity, public Resource<RenderEntity>
	{
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


}


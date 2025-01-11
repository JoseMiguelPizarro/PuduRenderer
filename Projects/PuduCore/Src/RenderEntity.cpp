#include "RenderEntity.h"
#include "Scene.h"

namespace Pudu {

	Model* RenderEntity::GetModel()
	{
		return &m_model;
	}

	void RenderEntity::SetModel(Model& model)
	{
		m_model = model;
	}
	void RenderEntity::AttatchToScene(Scene& scene)
	{
		scene.AddRendererEntity(dynamic_pointer_cast<RenderEntity>(m_entitySPtr));
	}

	RenderSettings& RenderEntity::GetRenderSettings()
	{
		return this->m_renderSettings;
	}
}

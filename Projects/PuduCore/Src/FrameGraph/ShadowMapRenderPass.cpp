#include "FrameGraph/ShadowMapRenderPass.h"
#include "Resources/Resources.h"

namespace Pudu {
	void ShadowMapRenderPass::Initialize(PuduGraphics* gfx)
	{
		m_renderCamera = {};
		m_renderCamera.projectionType = Camera::ProjectionType::Ortographic;
		m_renderCamera.Width = 20;
		m_renderCamera.Height = 20;
	}
	void ShadowMapRenderPass::PreRender(RenderFrameData& renderData)
	{
		m_previousCamera = renderData.camera;
		auto dir = renderData.scene->directionalLight->direction;
		m_renderCamera.Transform.SetForward(dir, { 0,1,0 });
		m_renderCamera.Transform.LocalPosition = dir * 1000.0f;

		renderData.camera = &m_renderCamera;
	}

	void ShadowMapRenderPass::AfterRender(RenderFrameData& renderData)
	{
		renderData.camera = m_previousCamera;
	}
	RenderPassType ShadowMapRenderPass::GetRenderPassType()
	{
		return RenderPassType::ShadowMap; 
	}
}


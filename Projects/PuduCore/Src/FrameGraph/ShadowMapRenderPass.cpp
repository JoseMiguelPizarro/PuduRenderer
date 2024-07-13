#include "FrameGraph/ShadowMapRenderPass.h"
#include "Resources/Resources.h"

namespace Pudu {
	void ShadowMapRenderPass::Initialize(PuduGraphics* gfx)
	{
		m_renderCamera = {};
		m_renderCamera.projectionType = Camera::ProjectionType::Ortographic;
		m_renderCamera.Width = 5;
		m_renderCamera.Height = 5;
		
	}
	void ShadowMapRenderPass::PreRender(RenderFrameData& renderData)
	{
		InitializeShadowMap(renderData.graphics);

		m_previousCamera = renderData.camera;
		auto dir = renderData.scene->directionalLight->direction;
		m_renderCamera.Transform.SetForward(dir, { 0,1,0 });
		m_renderCamera.Transform.LocalPosition = -dir * 10.0f;

		renderData.camera = &m_renderCamera;

	//	renderData.currentCommand->TransitionImageLayout(m_shadowmap->vkImageHandle, m_shadowmap->format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	void ShadowMapRenderPass::AfterRender(RenderFrameData& renderData)
	{
		renderData.camera = m_previousCamera;

		//renderData.currentCommand->TransitionImageLayout(m_shadowmap->vkImageHandle, m_shadowmap->format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	RenderPassType ShadowMapRenderPass::GetRenderPassType()
	{
		return RenderPassType::ShadowMap; 
	}
	void ShadowMapRenderPass::InitializeShadowMap(PuduGraphics* gfx)
	{
		if (m_shadowmap == nullptr)
		{
			m_shadowmap = gfx->Resources()->GetTextureByName(K_SHADOWMAP_NAME);
		}
	}
}


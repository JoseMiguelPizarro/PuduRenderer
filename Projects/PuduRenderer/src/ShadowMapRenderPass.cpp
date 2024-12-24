#include "ShadowMapRenderPass.h"
#include "Resources/Resources.h"

namespace Pudu {
	void ShadowMapRenderPass::Initialize(PuduGraphics* gfx)
	{
		m_renderCamera = {};
	}
	void ShadowMapRenderPass::PreRender(RenderFrameData& renderData)
	{
		RenderPass::PreRender(renderData);

		InitializeShadowMap(renderData.graphics);

		m_previousCamera = renderData.camera;
		auto dirLight = renderData.scene->directionalLight;
		auto dir = dirLight->Direction();
		m_renderCamera.Transform.SetForward(dir, { 0,1,0 });
		m_renderCamera.Transform.SetLocalPosition(dirLight->GetTransform().GetLocalPosition());
		m_renderCamera.Projection = renderData.scene->directionalLight->Projection;

		renderData.camera = &m_renderCamera;
	}

	void ShadowMapRenderPass::AfterRender(RenderFrameData& renderData)
	{
		renderData.camera = m_previousCamera;
	}
	void ShadowMapRenderPass::Render(RenderFrameData& frameData)
	{
		frameData.currentCommand->SetDepthBias(m_depthBiasSlope, m_depthConstantBias);
		RenderPass::Render(frameData);
	}
	
	void ShadowMapRenderPass::InitializeShadowMap(PuduGraphics* gfx)
	{
		if (m_shadowmap == nullptr)
		{
			m_shadowmap = gfx->Resources()->GetTextureByName(K_SHADOWMAP_NAME);
		}
	}
}


#pragma once
#include <unordered_map>
#include "Scene.h"
#include "Renderer.h"
#include "PuduGraphics.h"
#include <FrameGraph/FrameGraph.h>
#include "ForwardRenderPass.h"
#include "DepthStencilRenderPass.h"
#include "ShadowMapRenderPass.h"
#include <PuduApp.h>
#include "ImguiRenderPass.h"

namespace Pudu
{
	class PuduRenderer :public Renderer
	{
	public:
		void SetSkyBox();

	protected:
		void OnInit(PuduGraphics* graphics, PuduApp* app) override;
		virtual void OnRender(RenderFrameData& data) override;

	private:
		void UpdateLightingBuffer(RenderFrameData& data) const;
		void UpdateGlobalConstantsBuffer(RenderFrameData& frame) const;
		void InitLightingBuffer(PuduGraphics* graphics);
		void InitConstantsBuffer(PuduGraphics* graphics);

	private:
		SPtr<RenderPass> m_depthRenderPass;
		SPtr<RenderPass> m_forwardRenderPass;
		SPtr<RenderPass> m_shadowMapRenderPass;
		SPtr<RenderPass> m_postProcessingRenderPass;
		SPtr<ImguiRenderPass> m_imguiRenderPass;
		SPtr<GraphicsBuffer> m_lightingBuffer;
		SPtr<GraphicsBuffer> m_globalConstantsBuffer;
		SPtr<Material> m_globalPropertiesMaterial;
	};
}


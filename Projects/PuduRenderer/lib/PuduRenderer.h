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

namespace Pudu
{
	class PuduRenderer :public Renderer
	{
	public:
		void SetSkyBox();

	protected:
		void OnInit(PuduGraphics* graphics, PuduApp* app) override;

	private:
		SPtr<RenderPass> m_depthRenderPass;
		SPtr<RenderPass> m_forwardRenderPass;
		SPtr<RenderPass> m_shadowMapRenderPass;
		SPtr<RenderPass> m_postProcessingRenderPass;

	};
}


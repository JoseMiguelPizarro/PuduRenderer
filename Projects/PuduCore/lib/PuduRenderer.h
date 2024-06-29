#pragma once
#include <unordered_map>
#include "Scene.h"
#include "PuduGraphics.h"
#include <FrameGraph/FrameGraph.h>
#include "FrameGraph/ForwardRenderPass.h"
#include "FrameGraph/DepthStencilRenderPass.h"
#include "FrameGraph/ShadowMapRenderPass.h"

#include <PuduApp.h>

namespace Pudu
{
	class PuduRenderer
	{
	public:
		Scene* sceneToRender;
		PuduGraphics* graphics;
		FrameGraph frameGraph;
		FrameGraphBuilder frameGraphBuilder;
		Pipeline* pipeline;
		PuduApp* app;

		void Init(PuduGraphics* graphics, PuduApp* app);
		void Render();
		void LoadFrameGraph(fs::path path);
		Pipeline* GetOrCreatePipeline(RenderFrameData& data, RenderPassType renderPassType);
		void AddRenderPass(RenderPass* renderPass, RenderPassType renderPasstype);

	private:
		DepthStencilRenderPass m_depthRenderPass;
		ForwardRenderPass m_forwardRenderPass;
		ShadowMapRenderPass m_shadowMapRenderPass;

		std::unordered_map<RenderPassType, std::unordered_map<SPtr<Shader>, PipelineHandle>> m_pipelinesByRenderPass;
		std::unordered_map<RenderPassType, RenderPass*> m_renderPassByType;
	};
}


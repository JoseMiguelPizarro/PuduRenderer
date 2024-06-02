#pragma once
#include <unordered_map>
#include "Scene.h"
#include "PuduGraphics.h"
#include <FrameGraph/FrameGraph.h>
#include "FrameGraph/ForwardRenderPass.h"
#include "FrameGraph/DepthStencilRenderPass.h"

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

		void Init(PuduGraphics* graphics);
		void Render();
		void LoadFrameGraph(fs::path path);
		Pipeline* GetOrCreatePipeline(RenderFrameData& data, RenderPassType renderPassType);
		void AddRenderPass(FrameGraphRenderPass* renderPass, RenderPassType renderPasstype);

	private:
		DepthStencilRenderPass m_depthRenderPass;
		ForwardRenderPass m_forwardRenderPass;

		std::unordered_map<RenderPassType, std::unordered_map<SPtr<Shader>, PipelineHandle>> m_pipelinesByRenderPass;
		std::unordered_map<RenderPassType, FrameGraphRenderPass*> m_renderPassByType;
	};
}


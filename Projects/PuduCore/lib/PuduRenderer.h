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
		void SetSkyBox();
		Pipeline* GetOrCreatePipeline(PipelineQueryData query);
		void AddRenderPass(RenderPass* renderPass, RenderPassType renderPasstype);

	private:
		DepthStencilRenderPass m_depthRenderPass;
		ForwardRenderPass m_forwardRenderPass;
		ShadowMapRenderPass m_shadowMapRenderPass;

		std::unordered_map<RenderPass*, std::unordered_map<Shader*, PipelineHandle>> m_pipelinesByRenderPass;

		Pipeline* CreatePipelineByRenderPassAndShader(RenderPass* renderPass, Shader* shader);
	};
}


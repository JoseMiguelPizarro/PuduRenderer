#pragma once
#include <unordered_map>
#include "Scene.h"
#include "PuduGraphics.h"
#include <FrameGraph/FrameGraph.h>

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

		void Init();
		void Render();
		void LoadFrameGraph(fs::path path);
		Pipeline* GetPipeline(DrawCall& drawcall);
		Pipeline* GetPipeline(DrawCall& drawcall, RenderPassType renderPassType);
		void AddRenderPass(FrameGraphRenderPass renderPass, RenderPassType renderPasstype);


	private:
		std::unordered_map<RenderPassType, std::unordered_map<SPtr<Shader>, Pipeline*>> m_pipelinesByRenderPass;
		std::unordered_map<RenderPassType, FrameGraphRenderPass> m_renderPassByType;
	};
}


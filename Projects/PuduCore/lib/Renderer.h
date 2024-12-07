#pragma once
#include <unordered_map>
#include "Pipeline.h"
#include "RenderFrameData.h"
#include "FrameGraph/FrameGraph.h"

namespace Pudu {
	class RenderPass;
	class Pipeline;
	class Shader;

	class Renderer {
	public:
		Pipeline* GetOrCreatePipeline(PipelineQueryData query);

		Pipeline* CreatePipelineByRenderPassAndShader(RenderPass* renderPass, Shader* shader);

		void AddRenderPass(RenderPass* renderPass);

		void Init(PuduGraphics* graphics, PuduApp* app);

		void Render(Scene* scene);

	protected:
		PuduGraphics* graphics;
		PuduApp* app;

		virtual void OnInit(PuduGraphics* graphics, PuduApp* app) {};
		virtual void OnRender(RenderFrameData& data) {};

		FrameGraph frameGraph;
		FrameGraphBuilder frameGraphBuilder;
		std::unordered_map < RenderPass*, std::unordered_map<Shader*, Pipeline*>> m_pipelinesByRenderPass;
	};
}
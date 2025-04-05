#include <unordered_map>
#include <vector>
#include <Logger.h>
#include "Renderer.h"
#include "FrameGraph/RenderPass.h"
#include "Shader.h"
#include "Pipeline.h"

namespace Pudu
{
	Pipeline* Renderer::GetOrCreatePipeline(PipelineQueryData query)
	{
		auto shader = query.shader;
		auto renderPass = query.renderPass;

		if (m_pipelinesByRenderPass.contains(renderPass))
		{
			auto renderPassPipelines = m_pipelinesByRenderPass.find(renderPass);
			if (renderPassPipelines != m_pipelinesByRenderPass.end())
			{
				auto pipelinesByShader = renderPassPipelines->second;

				auto pipeline = pipelinesByShader.find(shader);
				if (pipeline != pipelinesByShader.end())
				{
					return pipeline->second;
				}
				else
				{
					return CreatePipelineByRenderPassAndShader(renderPass, shader);
				}
			}
		}
		//Create new graphic pipeline
		else
		{
			return CreatePipelineByRenderPassAndShader(renderPass, shader);
		}

		return nullptr;
	}

	Pipeline* Renderer::CreatePipelineByRenderPassAndShader(RenderPass* renderPass, IShaderObject* shader)
	{
		auto pipeline = shader->CreatePipeline(graphics, renderPass).get();

		//Renderpass already cached
		if (m_pipelinesByRenderPass.contains(renderPass))
		{
			auto pipelineByShader = &m_pipelinesByRenderPass.find(renderPass)->second;
			pipelineByShader->insert(std::make_pair(shader, pipeline));
		}
		//Create new cache
		else
		{
			std::unordered_map<IShaderObject*, Pipeline*> pipelineByShaderMap;
			pipelineByShaderMap.insert(std::make_pair(shader, pipeline));

			m_pipelinesByRenderPass.insert(std::make_pair(renderPass, pipelineByShaderMap));
		}

		return pipeline;
	}

	void Renderer::AddRenderPass(RenderPass* renderPass)
	{
		renderPass->Create(graphics);
		renderPass->Initialize(graphics);

		FrameGraphNodeCreation frameGraphNode;
		frameGraphNode.name = renderPass->name;
		frameGraphNode.renderPass = renderPass->Handle();
		frameGraphNode.enabled = true;

		frameGraph.CreateNode(frameGraphNode);
	}

	void Renderer::Init(PuduGraphics* graphics, PuduApp* app)
	{
		frameGraph = FrameGraph();
		frameGraphBuilder = FrameGraphBuilder();
		frameGraphBuilder.Init(graphics);
		frameGraph.Init(&frameGraphBuilder);

		this->graphics = graphics;
		this->app = app;

		OnInit(graphics, app);
	}

	void Renderer::Render(Scene* scene)
	{
		RenderFrameData renderData{};

		renderData.renderer = this;
		renderData.scene = scene;
		renderData.frameGraph = &frameGraph;
		renderData.graphics = graphics;
		renderData.app = app;

		SetRenderCamera(scene->camera);

		renderData.camera = scene->camera;
		graphics->DrawFrame(renderData);
	}

	Camera* Renderer::GetRenderCamera()
	{
		return m_renderCamera;
	}

	void Renderer::UploadCameraData(RenderFrameData& frameData)
	{
		if (m_isRenderCameraDirty)
		{
			OnUploadCameraData(frameData);

			m_isRenderCameraDirty = false;
		}
	}
}


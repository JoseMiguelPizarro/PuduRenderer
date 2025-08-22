#include <unordered_map>
#include <vector>
#include "Renderer.h"
#include "FrameGraph/RenderPass.h"
#include "Pipeline.h"
#include "RenderEntity.h"

namespace Pudu
{
	Pipeline* Renderer::GetOrCreatePipeline(PipelineQueryData query)
	{
		auto shader = query.shader;
		auto renderPass = query.renderPass;

		if (shader->NeedsPipelineRebuild())
		{

		}

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

		PrepareDrawcalls(renderData);

		SetRenderCamera(scene->camera);

		renderData.camera = scene->camera;
		graphics->DrawFrame(renderData);
	}

	Camera* Renderer::GetRenderCamera() const
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



	void Renderer::PrepareDrawcalls(RenderFrameData& frameData)
	{
		ClearDrawCalls(frameData);

		auto scene = frameData.scene;

		for (auto entity:scene->GetEntities())
		{
			RenderEntitySPtr renderEntity = std::dynamic_pointer_cast<RenderEntity>(entity);

			if (renderEntity != nullptr)
			{
				auto model = renderEntity->GetModel();
				for (size_t i = 0; i < model->Meshes.size(); i++)
				{
					auto mesh = model->Meshes[i];
					mesh->Create(frameData.graphics); //Ensure mesh has been GPU allocated

					SPtr<Material> material = model->Materials[0];
					if (i >= model->Materials.size())
					{
						material = model->Materials[0];
					}

					model->Transform = &renderEntity->GetTransform();
					DrawCall dc(model, model->Meshes[i], material);

					AddDrawCall(dc, renderEntity->GetRenderSettings());
				}
			}
		}
	}

	void Renderer::AddDrawCall(DrawCall& drawCall, RenderSettings& settings)
	{
		m_drawCallsPerLayer[settings.layer].push_back(drawCall);
	}

	void Renderer::ClearDrawCalls(RenderFrameData& frameData)
	{
		for (auto& layer : m_drawCallsPerLayer)
		{
			layer.clear();
		}
	}
}


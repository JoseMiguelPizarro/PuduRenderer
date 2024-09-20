#include <unordered_map>
#include <vector>
#include "PuduRenderer.h"
#include "SPIRVParser.h"
#include "FrameGraph/ForwardRenderPass.h"
#include "FrameGraph/DepthStencilRenderPass.h"
#include <Logger.h>
#include "Shader.h"

namespace Pudu
{
	void PuduRenderer::Init(PuduGraphics* graphics, PuduApp* app)
	{
		this->graphics = graphics;
		this->app = app;

		AddRenderPass(&m_depthRenderPass, RenderPassType::DepthPrePass);
		AddRenderPass(&m_forwardRenderPass, RenderPassType::Color);
		AddRenderPass(&m_shadowMapRenderPass, RenderPassType::ShadowMap);
	}
	void PuduRenderer::Render()
	{
		RenderFrameData renderData{};

		renderData.renderer = this;
		renderData.scene = sceneToRender;
		renderData.frameGraph = &frameGraph;
		renderData.graphics = graphics;
		renderData.app = app;

		graphics->DrawFrame(renderData);
	}

	void PuduRenderer::AddRenderPass(RenderPass* renderPass, RenderPassType renderPasstype)
	{
		renderPass->Initialize(graphics);
	}

	void PuduRenderer::LoadFrameGraph(std::filesystem::path path)
	{
		LOG("Loading FrameGraph");
		frameGraph = FrameGraph();
		frameGraphBuilder = FrameGraphBuilder();
		frameGraphBuilder.Init(graphics);
		frameGraph.Init(&frameGraphBuilder);

		frameGraph.Parse(path);
		frameGraph.AllocateRequiredResources();
		frameGraph.Compile();

		LOG("Loading FrameGraph End");
	}

	Pipeline* PuduRenderer::CreatePipelineByRenderPassAndShader(RenderPass* renderPass, Shader* shader)
	{
		PipelineCreationData creationData; //"Question now, how do we populate this?"
		creationData.vertexShaderData = shader->vertexData;
		creationData.fragmentShaderData = shader->fragmentData;
		creationData.name = renderPass->name.c_str();

		BlendStateCreation blendStateCreation;

		if (shader->HasFragmentData())
		{
			blendStateCreation.AddBlendState()
				.SetAlphaBlending(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD)
				.SetColorBlending(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD)
				.SetColorWriteMask(ColorWriteEnabled::All_mask);
		}

		RasterizationCreation rasterizationCreation;
		rasterizationCreation.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationCreation.fill = FillMode::Solid;
		rasterizationCreation.front = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		DepthStencilCreation depthStencilCreation;
		depthStencilCreation.SetDepth(true, VK_COMPARE_OP_LESS_OR_EQUAL);

		VertexInputCreation vertexInputCreation;
		auto attribDescriptions = Vertex::GetAttributeDescriptions();
		auto bindingDescriptions = Vertex::GetBindingDescription();

		for (auto attrib : attribDescriptions)
		{
			VertexAttribute a;
			a.binding = attrib.binding;
			a.format = attrib.format;
			a.location = attrib.location;
			a.offset = attrib.offset;

			vertexInputCreation.AddVertexAttribute(a);
		}

		VertexStream vertexStream;
		vertexStream.binding = bindingDescriptions.binding;
		vertexStream.inputRate = (VertexInputRate::Enum)bindingDescriptions.inputRate;
		vertexStream.stride = bindingDescriptions.stride;

		vertexInputCreation.AddVertexStream(vertexStream);

		ShaderStateCreationData shaderData;
		shaderData.SetName(shader->name.c_str());

		if (shader->HasFragmentData())
		{
			shaderData.AddStage(&shader->fragmentData, shader->fragmentData.size() * sizeof(char), VK_SHADER_STAGE_FRAGMENT_BIT);
		}

		if (shader->HasVertexData())
		{
			shaderData.AddStage(&shader->vertexData, shader->vertexData.size() * sizeof(char), VK_SHADER_STAGE_VERTEX_BIT);
		}

		creationData.descriptorCreationData = shader->descriptors;
		creationData.blendState = blendStateCreation;
		creationData.rasterization = rasterizationCreation;
		creationData.depthStencil = depthStencilCreation;
		creationData.vertexInput = vertexInputCreation;
		creationData.shadersStateCreationData = shaderData;

		creationData.renderPassHandle = renderPass->handle;

		auto handle = graphics->CreateGraphicsPipeline(creationData);
		Pipeline* pipeline = graphics->Resources()->GetPipeline(handle);

		//Renderpass already cached
		if (m_pipelinesByRenderPass.contains(renderPass))
		{
			auto pipelineByShader = &m_pipelinesByRenderPass.find(renderPass)->second;
			pipelineByShader->insert(std::make_pair(shader, handle));
		}
		//Create new cache
		else
		{
			std::unordered_map<Shader*, PipelineHandle> pipelineByShaderMap;
			pipelineByShaderMap.insert(std::make_pair(shader, handle));

			m_pipelinesByRenderPass.insert(std::make_pair(renderPass, pipelineByShaderMap));
		}

		return pipeline;
	}

	Pipeline* PuduRenderer::GetOrCreatePipeline(PipelineQueryData query)
	{
		auto shader = query.shader;
		auto renderPass = query.renderPass;

		if (m_pipelinesByRenderPass.contains(renderPass)) {
			auto renderPassPipelines = m_pipelinesByRenderPass.find(renderPass);
			if (renderPassPipelines != m_pipelinesByRenderPass.end())
			{
				auto pipelinesByShader = renderPassPipelines->second;

				auto pipeline = pipelinesByShader.find(shader);
				if (pipeline != pipelinesByShader.end())
				{
					return graphics->Resources()->GetPipeline(pipeline->second);
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
}


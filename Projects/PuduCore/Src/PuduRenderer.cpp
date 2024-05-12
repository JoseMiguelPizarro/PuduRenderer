#include <unordered_map>
#include <vector>
#include "PuduRenderer.h"
#include "SPIRVParser.h"
#include "FrameGraph/ForwardRenderPass.h"
#include "FrameGraph/DepthStencilRenderPass.h"
#include <Logger.h>

namespace Pudu
{
	void PuduRenderer::Init(PuduGraphics* graphics)
	{
		this->graphics = graphics;

		DepthStencilRenderPass depthPass;
		ForwardRenderPass forwardPass;

		AddRenderPass(depthPass, RenderPassType::DepthPrePass);
		AddRenderPass(forwardPass, RenderPassType::Color);
	}
	void PuduRenderer::Render()
	{
		RenderFrameData renderData{};

		renderData.renderer = this;
		renderData.scene = sceneToRender;
		renderData.frameGraph = &frameGraph;
		renderData.m_renderPassesByType = &m_renderPassByType;
		renderData.graphics = graphics;

		graphics->DrawFrame(renderData);
	}

	void PuduRenderer::AddRenderPass(FrameGraphRenderPass renderPass, RenderPassType renderPasstype)
	{
		m_renderPassByType.emplace(renderPasstype, renderPass);
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

	Pipeline* PuduRenderer::GetPipeline(RenderFrameData& frameData, RenderPassType renderPassType)
	{
		auto shader = frameData.currentDrawCall->MaterialPtr.Shader;

		if (renderPassType == DepthPrePass)
		{
			return m_pipelinesByRenderPass[renderPassType][0];
		}

		if (m_pipelinesByRenderPass.contains(renderPassType)) {
			auto renderPassPipelines = m_pipelinesByRenderPass.find(renderPassType);
			if (renderPassPipelines != m_pipelinesByRenderPass.end())
			{
				auto pipelinesByShader = renderPassPipelines->second;

				auto pipeline = pipelinesByShader.find(shader);
				if (pipeline != pipelinesByShader.end())
				{
					return pipeline->second;
				}
			}
		}
		//Create new graphic pipeline
		else
		{
			PipelineCreationData creationData; //"Question now, how do we populate this?"
			creationData.vertexShaderData = shader->vertexData;
			creationData.fragmentShaderData = shader->fragmentData;

			BlendStateCreation blendStateCreation;
			blendStateCreation.AddBlendState()
				.SetAlphaBlending(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD)
				.SetColorBlending(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD)
				.SetColorWriteMask(ColorWriteEnabled::All_mask);

			RasterizationCreation rasterizationCreation;
			rasterizationCreation.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizationCreation.fill = FillMode::Solid;
			rasterizationCreation.front = VK_FRONT_FACE_COUNTER_CLOCKWISE;

			DepthStencilCreation depthStencilCreation;
			depthStencilCreation.SetDepth(true, VK_COMPARE_OP_ALWAYS);

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
			shaderData.AddStage(&shader->fragmentData, shader->fragmentData.size() * sizeof(char), VK_SHADER_STAGE_FRAGMENT_BIT);
			shaderData.AddStage(&shader->vertexData, shader->vertexData.size() * sizeof(char), VK_SHADER_STAGE_VERTEX_BIT);

			SPIRVParser::GetDescriptorSetLayout(creationData, creationData.descriptorSetLayoutData);

			creationData.blendState = blendStateCreation;
			creationData.rasterization = rasterizationCreation;
			creationData.depthStencil = depthStencilCreation;
			creationData.vertexInput = vertexInputCreation;
			creationData.shadersStateCreationData = shaderData;

			creationData.renderPassHandle = frameData.currentRenderPass->handle;

			auto handle = graphics->CreateGraphicsPipeline(creationData);
			Pipeline* pipeline = graphics->Resources()->GetPipeline(handle);

			std::unordered_map<SPtr<Shader>, Pipeline*> pipelineByShaderMap;
			pipelineByShaderMap.insert(std::make_pair(shader, pipeline));

			m_pipelinesByRenderPass.insert(std::make_pair(renderPassType, pipelineByShaderMap));

			return pipeline;
		}
	}
}


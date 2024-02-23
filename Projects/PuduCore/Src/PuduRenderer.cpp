#include <unordered_map>
#include <vector>
#include "PuduRenderer.h"
#include "FrameGraph/ForwardRenderPass.h"
#include "FrameGraph/DepthStencilRenderPass.h"

namespace Pudu
{
	void PuduRenderer::Init()
	{
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

		graphics->DrawFrame(renderData);
	}

	void PuduRenderer::AddRenderPass(FrameGraphRenderPass renderPass, RenderPassType renderPasstype) 
	{
		m_renderPassByType.emplace(renderPasstype, renderPass);
	}

	void PuduRenderer::LoadFrameGraph(std::filesystem::path path)
	{
		frameGraph = FrameGraph();
		frameGraphBuilder = FrameGraphBuilder();
		frameGraphBuilder.Init(graphics);
		frameGraph.Init(&frameGraphBuilder);

		frameGraph.Parse(path);
		frameGraph.Compile();
	}
	Pipeline* PuduRenderer::GetPipeline(DrawCall& drawcall)
	{
		return nullptr;
	}

	Pipeline* PuduRenderer::GetPipeline(DrawCall& drawcall, RenderPassType renderPassType)
	{
		auto shader = drawcall.MaterialPtr.Shader;

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
				shaderData.AddStage(shader->fragmentData.data(), shader->fragmentData.size() * sizeof(char), VK_SHADER_STAGE_FRAGMENT_BIT);
				shaderData.AddStage(shader->vertexData.data(), shader->vertexData.size() * sizeof(char), VK_SHADER_STAGE_VERTEX_BIT);

				auto handle = graphics->CreateGraphicsPipeline(creationData);
				Pipeline* pipeline = graphics->m_resources->GetPipeline(handle);

				std::unordered_map<SPtr<Shader>, Pipeline*> pipelineByShaderMap;
				pipelineByShaderMap.insert(std::make_pair(shader, pipeline));

				m_pipelinesByRenderPass.insert(std::make_pair(renderPassType, pipelineByShaderMap));

				return pipeline;
			}
		}

		return nullptr;
	}
}


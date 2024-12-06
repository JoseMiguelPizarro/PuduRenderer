#include <unordered_map>
#include <vector>
#include "PuduRenderer.h"
#include "SPIRVParser.h"
#include "FrameGraph/ForwardRenderPass.h"
#include "FrameGraph/DepthStencilRenderPass.h"
#include "FrameGraph/PostProcessingRenderPass.h"
#include <Logger.h>
#include "Shader.h"

namespace Pudu
{
	void PuduRenderer::Init(PuduGraphics* graphics, PuduApp* app)
	{
		this->graphics = graphics;
		this->app = app;

		frameGraph = FrameGraph();
		frameGraphBuilder = FrameGraphBuilder();
		frameGraphBuilder.Init(graphics);
		frameGraph.Init(&frameGraphBuilder);

		auto depthRT = graphics->GetRenderTexture();
		depthRT->depth = 1;
		depthRT->width = graphics->WindowWidth;
		depthRT->height = graphics->WindowHeight;
		depthRT->format = VK_FORMAT_D32_SFLOAT;
		depthRT->name = "DepthPrepassTexture";

		auto shadowRT = graphics->GetRenderTexture();
		shadowRT->depth = 1;
		shadowRT->width = graphics->WindowWidth;
		shadowRT->height = graphics->WindowHeight;
		shadowRT->format = VK_FORMAT_D16_UNORM;
		shadowRT->name = "ShadowMap";

		auto colorRT = graphics->GetRenderTexture();
		colorRT->depth = 1;
		colorRT->width = graphics->WindowWidth;
		colorRT->height = graphics->WindowHeight;
		colorRT->format = VK_FORMAT_R8G8B8A8_UNORM;
		colorRT->name = "ForwardColor";

		m_depthRenderPass = graphics->GetRenderPass<DepthPrepassRenderPass>();
		m_depthRenderPass->name = "DepthPrepassRenderPass";
		m_depthRenderPass->AddDepthStencilAttachment(depthRT, AttachmentUsage::Write, LoadOperation::Clear);

		m_shadowMapRenderPass = graphics->GetRenderPass<ShadowMapRenderPass>();
		m_shadowMapRenderPass->name = "ShadowMapRenderPass";
		m_shadowMapRenderPass->AddDepthStencilAttachment(shadowRT, AttachmentUsage::Write, LoadOperation::Clear);

		m_forwardRenderPass = graphics->GetRenderPass<ForwardRenderPass>();
		m_forwardRenderPass->name = "ForwardRenderPass";
		m_forwardRenderPass->AddColorAttachment(colorRT, AttachmentUsage::Write, LoadOperation::Clear);
		m_forwardRenderPass->AddColorAttachment(shadowRT, AttachmentUsage::Read, LoadOperation::Load);
		m_forwardRenderPass->AddDepthStencilAttachment(depthRT, AttachmentUsage::Read, LoadOperation::Load);

		m_postProcessingRenderPass = graphics->GetRenderPass<PostProcessingRenderPass>();
		m_postProcessingRenderPass->name = "Postprocessing";
		m_postProcessingRenderPass->AddColorAttachment(colorRT, AttachmentUsage::Write, LoadOperation::Load);


		AddRenderPass(m_depthRenderPass.get());
		AddRenderPass(m_shadowMapRenderPass.get());
		AddRenderPass(m_forwardRenderPass.get());
	//	AddRenderPass(m_postProcessingRenderPass.get());

		frameGraph.AllocateRequiredResources();
		frameGraph.Compile();

		std::printf(frameGraph.ToString().c_str());
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

	void PuduRenderer::AddRenderPass(RenderPass* renderPass)
	{
		renderPass->Initialize(graphics);

		FrameGraphNodeCreation frameGraphNode;
		frameGraphNode.name = renderPass->name;
		frameGraphNode.renderPass = renderPass->Handle();
		frameGraphNode.enabled = true;

		frameGraph.CreateNode(frameGraphNode);
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
		depthStencilCreation.SetDepth(renderPass->writeDepth, VK_COMPARE_OP_LESS_OR_EQUAL);

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
			shaderData.AddStage(&shader->fragmentData, shader->fragmentData.size() * sizeof(char),
				VK_SHADER_STAGE_FRAGMENT_BIT);
		}

		if (shader->HasVertexData())
		{
			shaderData.AddStage(&shader->vertexData, shader->vertexData.size() * sizeof(char),
				VK_SHADER_STAGE_VERTEX_BIT);
		}

		creationData.descriptorCreationData = shader->descriptors;
		creationData.blendState = blendStateCreation;
		creationData.rasterization = rasterizationCreation;
		creationData.depthStencil = depthStencilCreation;
		creationData.vertexInput = vertexInputCreation;
		creationData.shadersStateCreationData = shaderData;

		creationData.renderPassHandle = renderPass->Handle();

		auto handle = graphics->CreateGraphicsPipeline(creationData);
		auto pipeline = graphics->Resources()->GetPipeline(handle).get();

		//Renderpass already cached
		if (m_pipelinesByRenderPass.contains(renderPass))
		{
			auto pipelineByShader = &m_pipelinesByRenderPass.find(renderPass)->second;
			pipelineByShader->insert(std::make_pair(shader, pipeline));
		}
		//Create new cache
		else
		{
			std::unordered_map<Shader*, Pipeline*> pipelineByShaderMap;
			pipelineByShaderMap.insert(std::make_pair(shader, pipeline));

			m_pipelinesByRenderPass.insert(std::make_pair(renderPass, pipelineByShaderMap));
		}

		return pipeline;
	}

	Pipeline* PuduRenderer::GetOrCreatePipeline(PipelineQueryData query)
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
}

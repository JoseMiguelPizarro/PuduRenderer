#include "FrameGraph/RenderPass.h"
#include "RenderFrameData.h"
#include "DrawCall.h"
#include "Scene.h"
#include "PuduGraphics.h"
#include "Resources/Resources.h"
#include "SPIRVParser.h"
#include <GPUCommands.h>

#include "Material.h"

#include "Pipeline.h"
#include "Renderer.h"

#include <Lighting/LightBuffer.h>


namespace Pudu
{
#pragma region attachments
	static VkRenderingAttachmentInfo RenderPassAttachmentToVKAttachment(RenderPassAttachment& attachment)
	{
		/*assert(
			attachment.resource->IsAllocated() && fmt::format("Texture {} is not allocated", attachment.resource->name).
			c_str());*/

		VkRenderingAttachmentInfo renderingAttachment = {};
		renderingAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		renderingAttachment.imageLayout = attachment.layout;
		renderingAttachment.clearValue = attachment.clearValue;
		renderingAttachment.imageView = attachment.resource->vkImageViewHandle;
		renderingAttachment.loadOp = attachment.loadOperation;
		renderingAttachment.storeOp = attachment.storeOp;

		return renderingAttachment;
	}


	RenderPassAttachments& RenderPassAttachments::Reset()
	{
		colorAttachmentCount = 0;
		depthStencilFormat = VK_FORMAT_UNDEFINED;
		depthOperation = stencilOperation = LoadOperation::DontCare;
		return *this;
	}

	RenderPassAttachments& RenderPassAttachments::AddColorAttachment(RenderPassAttachment attachment)
	{
		colorAttachments[colorAttachmentCount++] = attachment;

		if (attachment.usage & AttachmentUsage::Write)
		{
			colorAttachmentVkCount++;
		}

		return *this;
	}

	RenderPassAttachments& RenderPassAttachments::SetDepthStencilAttachment(RenderPassAttachment attachment)
	{
		depthStencilFormat = attachment.resource->format;

		depthAttachments[depthAttachmentCount++] = attachment;

		LoadOperation depthOperation;
		LoadOperation stencilOperation;

		switch (attachment.loadOperation)
		{
		case VK_ATTACHMENT_LOAD_OP_LOAD:
			depthOperation = LoadOperation::Load;
			stencilOperation = LoadOperation::Load;
			break;
		case VK_ATTACHMENT_LOAD_OP_CLEAR:
			depthOperation = LoadOperation::Clear;
			stencilOperation = LoadOperation::Clear;
			break;
		default:
			depthOperation = LoadOperation::DontCare;
			stencilOperation = LoadOperation::DontCare;
			break;
		}

		SetDepthStencilOperations(depthOperation, stencilOperation);
		depthStencilFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		if (attachment.usage != AttachmentUsage::Sample)
		{
			depthAttachmentVkCount++;
		}

		return *this;
	}

	RenderPassAttachments& RenderPassAttachments::SetDepthStencilOperations(
		LoadOperation depth, LoadOperation stencil)
	{
		depthOperation = depth;
		stencilOperation = stencil;

		return *this;
	}

	RenderPassAttachments& RenderPassAttachments::AddBufferAttachment(SPtr<GraphicsBuffer> buffer, AttachmentUsage usage)
	{
		RenderPassAttachment attachment;
		attachment.type = GPUResourceType::Buffer;
		attachment.buffer = buffer;
		bufferAttachments[buffersCount++] = attachment;

		return *this;
	}

	VkFormat RenderPassAttachments::GetStencilFormat()
	{
		return VK_FORMAT_UNDEFINED;
		if (stencilOperation == LoadOperation::DontCare)
		{
		}
	}

	uint16_t RenderPassAttachments::AttachmentCount()
	{
		return colorAttachmentCount + depthAttachmentCount;
	}

	uint16_t RenderPassAttachments::ColorAttachmentCount()
	{
		return colorAttachmentCount;
	}

	uint16_t RenderPassAttachments::BufferCount()
	{
		return buffersCount;
	}

	VkRenderingAttachmentInfo* RenderPassAttachments::GetVkColorAttachments()
	{
		if (m_VkcolorAttachmentsCreated)
		{
			return m_vkcolorAttachments;
		}

		colorAttachmentVkCount = 0;

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			auto attachment = colorAttachments[i];

			if (attachment.usage & AttachmentUsage::Write)
			{
				m_vkcolorAttachments[colorAttachmentVkCount] = RenderPassAttachmentToVKAttachment(attachment);
				colorAttachmentVkCount++;
			}
		}

		m_VkcolorAttachmentsCreated = true;

		return m_vkcolorAttachments;
	}

	std::vector<RenderPassAttachment>* RenderPassAttachments::GetColorRenderPassAttachments()
	{
		if (m_colorRenderPassAttachmentsCreated)
		{
			return &m_colorRenderPassAttachments;
		}

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			auto attachment = colorAttachments[i];

			if (attachment.usage & AttachmentUsage::Write)
			{
				m_colorRenderPassAttachments.push_back(attachment);
			}
		}

		return &m_colorRenderPassAttachments;
	}

	VkRenderingAttachmentInfo* RenderPassAttachments::GetDepthVkAttachments()
	{
		if (m_depthAttachmentsCreated)
		{
			return m_vkDepthAttachments;
		}

		depthAttachmentVkCount = 0;

		for (size_t i = 0; i < depthAttachmentCount; i++)
		{
			auto attachment = depthAttachments[i];

			if (attachment.usage != AttachmentUsage::Sample)
			{
				m_vkDepthAttachments[depthAttachmentVkCount] = RenderPassAttachmentToVKAttachment(attachment);
				depthAttachmentVkCount++;
			}
		}

		m_depthAttachmentsCreated = true;

		return m_vkDepthAttachments;
	}

	VkRenderingAttachmentInfo* RenderPassAttachments::GetStencilAttachments()
	{
		return nullptr;
	}

	VkFormat* RenderPassAttachments::GetColorAttachmentsFormat()
	{
		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			auto attachment = colorAttachments[i];

			if (attachment.usage & AttachmentUsage::Write)
			{
				m_colorAttachmentsFormat[i] = attachment.resource->format;
			}
		}

		return m_colorAttachmentsFormat;
	}

#pragma endregion


	Pipeline* RenderPass::GetPipeline(PipelineQueryData pipelineQuery)
	{
		return pipelineQuery.renderer->GetOrCreatePipeline(pipelineQuery);
	}

	void RenderPass::PreRender(RenderFrameData& renderData)
	{

	}

	void RenderPass::Render(RenderFrameData& frameData)
	{
		auto renderScene = frameData.scene;
		auto commands = frameData.currentCommand;

		auto drawCalls = renderScene->GetDrawCalls(m_renderLayer);

		for (DrawCall drawCall : *drawCalls)
		{
			frameData.currentDrawCall = &drawCall;

			BeforeRenderDrawcall(frameData, drawCall);

			auto model = drawCall.ModelPtr;
			auto mesh = drawCall.MeshPtr;

			auto material = GetRenderMaterial(frameData);

			Pipeline* pipeline = GetPipeline({
				.renderPass = frameData.currentRenderPass.get(),
				.shader = material->m_shader.get(),
				.renderer = frameData.renderer
			});

			if (pipeline != frameData.currentPipeline)
			{
				commands->BindPipeline(pipeline);
				frameData.currentPipeline = pipeline;
			}

			if (frameData.descriptorSetOffset > 0)
			{
				auto globalMaterial = frameData.globalPropertiesMaterial;
				commands->BindDescriptorSet(pipeline->vkPipelineLayoutHandle, globalMaterial->GetDescriptorSets(),
				                            frameData.descriptorSetOffset);
			}

			for (auto& mat : model->Materials)
			{
				mat->ApplyProperties();
			}

			if (pipeline->numActiveLayouts - frameData.descriptorSetOffset > 0)
			{
				BindMaterialDescriptorSets(pipeline, material, frameData);
			}

			commands->BindMesh(mesh.get());

			auto ubo = frameData.graphics->GetUniformBufferObject(frameData.camera, drawCall);

			Viewport viewport;
			viewport.rect = {0, 0, (uint16)frameData.graphics->WindowWidth, (uint16)frameData.graphics->WindowHeight};
			viewport.maxDepth = 1;
			commands->SetViewport(viewport);
			commands->PushConstants(pipeline->vkPipelineLayoutHandle,
			                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
			                        sizeof(UniformBufferObject), &ubo);

			commands->DrawIndexed(static_cast<uint32_t>(mesh->GetIndices()->size()), 1, 0, 0, 0);

			AfterRenderDrawcall(frameData, drawCall);
		}
	}

	void RenderPass::SetupRender(RenderFrameData& renderData)
	{
		auto commands = renderData.currentCommand;
		commands->SetDepthBias(0, 0);
		commands->SetScissor(0, 0, renderData.width, renderData.height);
		commands->SetViewport({ {0,0,renderData.width,renderData.height},0,1 });

		SPtr<RenderTexture> renderTarget;
		if (attachments.ColorAttachmentCount())
		{
			renderTarget = attachments.GetColorRenderPassAttachments()->at(0).resource;
		}
		else if (attachments.depthAttachmentCount)
		{
			renderTarget = attachments.depthAttachments[0].resource;
		}

		renderData.activeRenderTarget = renderTarget;
	}

	void RenderPass::SetComputeShader(ComputeShader* shader)
	{
		m_computeShader = shader;
	}

	ComputeShader* RenderPass::GetComputeShader()
	{
		return m_computeShader;
	}

	void RenderPass::OnCreate(PuduGraphics* gpu)
	{
		gpu->CreateRenderPass(this);
	}

	void RenderPass::BindMaterialDescriptorSets(Pipeline* pipeline, SPtr<Material> material,
		RenderFrameData& frameData)
	{
		if (pipeline->numActiveLayouts - frameData.descriptorSetOffset > 0)
		{
			frameData.currentCommand->BindDescriptorSet(pipeline->vkPipelineLayoutHandle,
				&material->GetDescriptorSets()[frameData.descriptorSetOffset],
				material->GetShader()->GetActiveLayoutCount() - frameData.descriptorSetOffset, frameData.descriptorSetOffset);
		}
	}

	SPtr<Material> RenderPass::GetRenderMaterial(const RenderFrameData& frameData) const
	{
		if (frameData.currentDrawCall->HasReplacementMaterial())
		{
			return frameData.currentDrawCall->GetRenderMaterial();
		}
		if (this->HasReplacementMaterial())
		{
			return GetReplacementMaterial();
		}

		return frameData.currentDrawCall->GetRenderMaterial();
	}

	VkRenderingInfo RenderPass::GetRenderingInfo(RenderFrameData& data)
	{
		VkRenderingInfo renderInfo = {};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = renderArea;
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = attachments.colorAttachmentVkCount;
		renderInfo.pColorAttachments = attachments.colorAttachmentVkCount > 0 ? attachments.GetVkColorAttachments() : nullptr;
		renderInfo.pDepthAttachment = attachments.depthAttachmentVkCount > 0 ? attachments.GetDepthVkAttachments()
			: nullptr;


		renderInfo.pStencilAttachment = nullptr;

		renderInfo.renderArea = { 0, 0, data.width, data.height };

		return renderInfo;
	}

	void RenderPass::BeginRender(RenderFrameData& data)
	{
		data.currentCommand->BegingRenderingPass(GetRenderingInfo(data));
	}

	void RenderPass::EndRender(RenderFrameData& data)
	{
		data.currentCommand->EndRenderingPass();
	}

	RenderPass* RenderPass::AddColorAttachment(SPtr<RenderTexture> rt, AttachmentUsage usage, LoadOperation loadOp,
		vec4 clearColor)
	{
		RenderPassAttachment attachment = {};
		attachment.resource = rt;
		attachment.loadOperation = ToVk(loadOp);
		attachment.storeOp = ToVk(usage);

		VkClearValue clear;
		clear.color = { {clearColor.x, clearColor.y, clearColor.z, clearColor.w} };

		attachment.clearValue = clear;
		attachment.usage = usage;

		return AddColorAttachment(attachment);
	}

	RenderPass* RenderPass::AddDepthStencilAttachment(SPtr<RenderTexture> rt, AttachmentUsage usage, LoadOperation loadOp,
		float depthClear, uint32_t stencilClear)
	{
		RenderPassAttachment attachment = {};
		attachment.resource = rt;
		attachment.loadOperation = ToVk(loadOp);
		attachment.storeOp = ToVk(usage);

		VkClearValue clear;
		clear.depthStencil = { depthClear, stencilClear };

		attachment.clearValue = clear;
		attachment.usage = usage;

		writeDepth = usage & AttachmentUsage::Write;

		return AddDepthStencilAttachment(attachment);
	}

	RenderPass* RenderPass::AddColorAttachment(RenderPassAttachment& attachment)
	{
		if (TextureFormat::IsDepthStencil(attachment.resource->format))
		{
			attachment.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		attachments.AddColorAttachment(attachment);

		return this;
	}

	RenderPass* RenderPass::AddDepthStencilAttachment(RenderPassAttachment& attachment)
	{
		attachment.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
		attachments.depthStencilFormat = attachment.resource->format;
		attachments.SetDepthStencilAttachment(attachment);

		return this;
	}

	RenderPass* RenderPass::AddBufferAttachment(SPtr<GraphicsBuffer> buffer, AttachmentUsage usage)
	{
		attachments.AddBufferAttachment(buffer, usage);

		return this;
	}

	RenderPass* RenderPass::SetName(const char* name)
	{
		this->name.append(name);

		return this;
	}
	RenderPass* RenderPass::SetCullMode(CullMode cullMode)
	{
		m_cullMode = cullMode;
		return this;
	}
	RenderPass* RenderPass::SetColorBlending(VkBlendFactor sourceColor, VkBlendFactor destinationColor, VkBlendOp colorOperation)
	{
		m_blendState.SetColorBlending(sourceColor, destinationColor, colorOperation);

		return this;
	}
	RenderPass* RenderPass::SetAlphaBlending(VkBlendFactor sourceAlpha, VkBlendFactor destinationAlpha, VkBlendOp alphaOperation)
	{
		m_blendState.SetAlphaBlending(sourceAlpha, destinationAlpha, alphaOperation);
		return this;
	}

	RenderPass* RenderPass::SetReplacementMaterial(SPtr<Material> material)
	{
		m_replacementMaterial = material;

		return this;
	}

	RenderPass* RenderPass::SetRenderLayer(uint32_t layer)
	{
		m_renderLayer = layer;

		return this;
	}

	SPtr<Material> RenderPass::GetReplacementMaterial() const
	{
		return m_replacementMaterial;
	}

	bool RenderPass::HasReplacementMaterial() const
	{
		return m_replacementMaterial != nullptr;
	}

	BlendState* RenderPass::GetBlendState()
	{
		return &this->m_blendState;
	}
	CullMode RenderPass::GetCullMode()
	{
		return m_cullMode;
	}
}

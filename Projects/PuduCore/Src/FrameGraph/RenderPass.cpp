#include "FrameGraph/RenderPass.h"
#include "RenderFrameData.h"
#include "DrawCall.h"
#include "Scene.h"
#include "PuduGraphics.h"
#include "Resources/Resources.h"
#include "SPIRVParser.h"
#include <GPUCommands.h>
#include "Lighting/Light.h"
#include "Material.h"
#include <stdexcept>
#include "Pipeline.h"
#include "Renderer.h"
#undef VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vk_enum_string_helper.h>
#define VK_USE_PLATFORM_WIN32_KHR
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

	VkRenderingAttachmentInfo* RenderPassAttachments::GetDepthAttachments()
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

	void RenderPass::Render(RenderFrameData& frameData)
	{
		auto renderScene = frameData.scene;
		auto commands = frameData.currentCommand;

		for (DrawCall drawCall : renderScene->GetDrawCalls())
		{
			frameData.currentDrawCall = &drawCall;

			BeforeRenderDrawcall(frameData, drawCall);

			Model model = drawCall.ModelPtr;
			auto mesh = drawCall.MeshPtr;

			Pipeline* pipeline = GetPipeline({
				.renderPass = frameData.currentRenderPass.get(),
				.shader = drawCall.GetRenderMaterial()->shader.get(),
				.renderer = frameData.renderer
				});
			if (pipeline != frameData.currentPipeline)
			{
				frameData.graphics->UpdateBindlessResources(pipeline);

				commands->BindPipeline(pipeline);
				frameData.currentPipeline = pipeline;

				if (pipeline->numActiveLayouts > 0)
				{
					//Bind Lighting Buffer
					VkDescriptorBufferInfo bufferInfo{};
					bufferInfo.buffer = frameData.lightingBuffer->vkHandle;
					bufferInfo.range = sizeof(LightBuffer);

					VkWriteDescriptorSet bufferWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
					bufferWrite.pNext = nullptr;

					bufferWrite.dstBinding = PuduGraphics::K_LIGHTING_BUFFER_BINDING;
					bufferWrite.dstSet = pipeline->vkDescriptorSets[1];
					bufferWrite.descriptorCount = 1;
					bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					bufferWrite.pBufferInfo = &bufferInfo;

					frameData.graphics->UpdateDescriptorSet(1, &bufferWrite);

					//Bind the per material properties here

					for (auto mat : model.Materials)
					{
						mat.GetPropertiesBlock()->ApplyProperties(frameData.graphics, frameData.currentDrawCall->GetRenderMaterial()->shader.get(), pipeline);
					}

					commands->BindDescriptorSet(pipeline->vkPipelineLayoutHandle, pipeline->vkDescriptorSets,
						pipeline->numDescriptorSets);
				}
			}

			commands->BindMesh(mesh.get());

			auto ubo = frameData.graphics->GetUniformBufferObject(frameData.camera, drawCall);

			Viewport viewport;
			viewport.rect = { 0, 0, (uint16)frameData.graphics->WindowWidth, (uint16)frameData.graphics->WindowHeight };
			viewport.maxDepth = 1;
			commands->SetViewport(viewport);
			vkCmdPushConstants(commands->vkHandle, pipeline->vkPipelineLayoutHandle,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
				sizeof(UniformBufferObject), &ubo);

			commands->DrawIndexed(static_cast<uint32_t>(mesh->GetIndices()->size()), 1, 0, 0, 0);

			AfterRenderDrawcall(frameData, drawCall);
		}
	}

	PipelineCreationData RenderPass::GetPipelineCreationData(RenderFrameData& frameData, DrawCall& drawcall)
	{
		PipelineCreationData creationData;

		auto shader = frameData.currentDrawCall->MaterialPtr.shader;

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
			shaderData.AddStage(&shader->fragmentData, shader->fragmentData.size() * sizeof(char),
				VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		if (shader->HasVertexData())
		{
			shaderData.AddStage(&shader->vertexData, shader->vertexData.size() * sizeof(char),
				VK_SHADER_STAGE_VERTEX_BIT);
		}

		creationData.descriptorCreationData = shader->GetDescriptorSetLayouts();
		creationData.blendState = blendStateCreation;
		creationData.rasterization = rasterizationCreation;
		creationData.depthStencil = depthStencilCreation;
		creationData.vertexInput = vertexInputCreation;
		creationData.shadersStateCreationData = shaderData;

		creationData.renderPassHandle = frameData.currentRenderPass->Handle();

		return creationData;
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

	VkRenderingInfo RenderPass::GetRenderingInfo(RenderFrameData& data)
	{
		VkRenderingInfo renderInfo = {};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = renderArea;
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = attachments.colorAttachmentVkCount;
		renderInfo.pColorAttachments = attachments.colorAttachmentVkCount > 0 ? attachments.GetVkColorAttachments() : nullptr;
		renderInfo.pDepthAttachment = attachments.depthAttachmentVkCount > 0 ? attachments.GetDepthAttachments()
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

	void RenderPass::AddColorAttachment(SPtr<RenderTexture> rt, AttachmentUsage usage, LoadOperation loadOp,
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

		AddColorAttachment(attachment);
	}

	void RenderPass::AddDepthStencilAttachment(SPtr<RenderTexture> rt, AttachmentUsage usage, LoadOperation loadOp,
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

		AddDepthStencilAttachment(attachment);
	}

	void RenderPass::AddColorAttachment(RenderPassAttachment& attachment)
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
	}

	void RenderPass::AddDepthStencilAttachment(RenderPassAttachment& attachment)
	{
		attachment.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
		attachments.depthStencilFormat = attachment.resource->format;
		attachments.SetDepthStencilAttachment(attachment);
	}

	void RenderPass::AddBufferAttachment(SPtr<GraphicsBuffer> buffer, AttachmentUsage usage)
	{
		attachments.AddBufferAttachment(buffer, usage);
	}

	void RenderPass::SetName(const char* name)
	{
		this->name.append(name);
	}
}

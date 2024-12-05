#include "FrameGraph/RenderPass.h"
#include "RenderFrameData.h"
#include "DrawCall.h"
#include "PuduRenderer.h"
#include "Resources/Resources.h"
#include "SPIRVParser.h"
#include <GPUCommands.h>
#include "Lighting/Light.h"
#include "Material.h"
#include <stdexcept>
#undef VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vk_enum_string_helper.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <Lighting/LightBuffer.h>


namespace Pudu
{
#pragma region attachments
	static VkRenderingAttachmentInfo RenderPassAttachmentToVKAttachment(RenderPassAttachment& attachment)
	{
		assert(
			attachment.resource->IsAllocated() && fmt::format("Texture {} is not allocated", attachment.resource->name).
			c_str());

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
		colorAttachmentsFormat[numColorFormats++] = attachment.resource->format;
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

	VkRenderingAttachmentInfo* RenderPassAttachments::GetColorAttachments()
	{
		if (m_colorAttachmentsCreated)
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

		m_colorAttachmentsCreated = true;

		return m_vkcolorAttachments;
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
				.shader = drawCall.GetRenderMaterial()->Shader.get(),
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
					bufferInfo.buffer = frameData.lightingBuffer->vkHandler;
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
						for (auto& request : mat.descriptorUpdateRequests)
						{
							//For this to work each material will need its own descriptorset that will have to be bind each frame
							if (!request.uploaded)
							{
								VkDescriptorImageInfo imageInfo{};
								imageInfo.imageView = request.texture->vkImageViewHandle;
								imageInfo.sampler = request.texture->Sampler.vkHandle;
								imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

								VkWriteDescriptorSet imageWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
								imageWrite.descriptorCount = 1;
								imageWrite.dstBinding = request.binding->index;
								imageWrite.dstSet = pipeline->vkDescriptorSets[request.binding->set];
								imageWrite.pImageInfo = &imageInfo;
								imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

								frameData.graphics->UpdateDescriptorSet(1, &imageWrite);
								request.uploaded = true;
							}
						}
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

		auto shader = frameData.currentDrawCall->MaterialPtr.Shader;

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

		creationData.descriptorCreationData = shader->descriptors;
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

	void RenderPass::Create(PuduGraphics* gpu)
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
		renderInfo.pColorAttachments = attachments.colorAttachmentVkCount > 0 ? attachments.GetColorAttachments() : nullptr;
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
		attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments.AddColorAttachment(attachment);
	}

	void RenderPass::AddDepthStencilAttachment(RenderPassAttachment& attachment)
	{
		attachment.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
		attachments.depthStencilFormat = attachment.resource->format;
		attachments.SetDepthStencilAttachment(attachment);

		LOG("Depth Attachment format {}", string_VkFormat(attachments.depthStencilFormat));
	}

	void RenderPass::SetName(const char* name)
	{
		this->name.append(name);
	}
}

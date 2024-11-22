#include "FrameGraph/RenderPass.h"
#include "RenderFrameData.h"
#include "DrawCall.h"
#include "PuduRenderer.h"
#include "Resources/Resources.h"
#include "SPIRVParser.h"
#include <GPUCommands.h>
#include "Lighting/Light.h"
#include "Material.h"

#include <Lighting/LightBuffer.h>


namespace Pudu
{
#pragma region attachments

	RenderPassAttachments& RenderPassAttachments::Reset()
	{
		colorAttachmentCount = 0;
		depthStencilFormat = VK_FORMAT_UNDEFINED;
		depthOperation = stencilOperation = RenderPassOperation::DontCare;
		return *this;
	}

	RenderPassAttachments& RenderPassAttachments::AddColorAttachment(RenderPassAttachment attachment)
	{
		VkRenderingAttachmentInfo attachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		attachmentInfo.clearValue = attachment.clearValue;
		attachmentInfo.loadOp = attachment.loadOperation;
		attachmentInfo.storeOp = attachment.storeOp;
		attachmentInfo.imageView = attachment.texture->vkImageViewHandle;
		attachmentInfo.imageLayout = attachment.layout;

		colorAttachmentsFormat[numColorFormats++] = attachment.texture->format;
		colorAttachments[colorAttachmentCount++] = attachmentInfo;

		return *this;
	}

	RenderPassAttachments& RenderPassAttachments::SetDepthStencilAttachment(RenderPassAttachment attachment)
	{
		VkRenderingAttachmentInfo attachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		attachmentInfo.clearValue = attachment.clearValue;
		attachmentInfo.loadOp = attachment.loadOperation;
		attachmentInfo.storeOp = attachment.storeOp;
		attachmentInfo.imageView = attachment.texture->vkImageViewHandle;
		attachmentInfo.imageLayout = attachment.layout;


		depthStencilFormat = attachment.texture->format;

		depthAttachments[depthAttachmentCount++] = attachmentInfo;

		return *this;
	}

	RenderPassAttachments& RenderPassAttachments::SetDepthStencilOperations(RenderPassOperation depth, RenderPassOperation stencil)
	{
		depthOperation = depth;
		stencilOperation = stencil;

		return *this;
	}
	VkFormat RenderPassAttachments::GetStencilFormat()
	{
		if (stencilOperation == RenderPassOperation::DontCare)
		{
			return VK_FORMAT_UNDEFINED;
		}
	}

	uint16_t RenderPassAttachments::AttachmentCount()
	{
		return colorAttachmentCount + depthAttachmentCount;
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

		for (DrawCall drawCall : renderScene->GetDrawCalls()) {

			frameData.currentDrawCall = &drawCall;

			BeforeRenderDrawcall(frameData, drawCall);

			Model model = drawCall.ModelPtr;
			auto mesh = drawCall.MeshPtr;

			Pipeline* pipeline = GetPipeline({ frameData.currentRenderPass.get(), drawCall.MaterialPtr.Shader.get(), frameData.renderer });
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

					for (auto mat : model.Materials) {
						for (auto& request : mat.descriptorUpdateRequests) {
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

					commands->BindDescriptorSet(pipeline->vkPipelineLayoutHandle, pipeline->vkDescriptorSets, pipeline->numDescriptorSets);
				}
			}

			VkBuffer vertexBuffers[] = { mesh->GetVertexBuffer()->vkHandler };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commands->vkHandle, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commands->vkHandle, mesh->GetIndexBuffer()->vkHandler, 0, VK_INDEX_TYPE_UINT32);

			auto ubo = frameData.graphics->GetUniformBufferObject(frameData.camera, drawCall);

			Viewport viewport;
			viewport.rect = { 0,0, (uint16)frameData.graphics->WindowWidth,(uint16)frameData.graphics->WindowHeight };
			viewport.maxDepth = 1;
			commands->SetViewport(viewport);
			vkCmdPushConstants(commands->vkHandle, pipeline->vkPipelineLayoutHandle, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(UniformBufferObject), &ubo);

			vkCmdDrawIndexed(commands->vkHandle, static_cast<uint32_t>(mesh->GetIndices()->size()), 1, 0, 0, 0);

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

		creationData.renderPassHandle = frameData.currentRenderPass->handle;

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

	VkRenderingInfo RenderPass::GetRenderingInfo(RenderFrameData& data)
	{
		VkRenderingInfo renderInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
		renderInfo.renderArea = renderArea;
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = attachments.colorAttachmentCount;
		renderInfo.pColorAttachments = attachments.colorAttachments;
		renderInfo.pDepthAttachment = attachments.depthAttachmentCount > 0 ? attachments.depthAttachments : nullptr;

		renderInfo.pStencilAttachment = nullptr;

		renderInfo.renderArea = { 0,0,data.width,data.height };

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

	void RenderPass::SetName(const char* name)
	{
		this->name.append(name);
	}
}

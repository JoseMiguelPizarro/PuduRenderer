#include "Resources/RenderPass.h"
#include "GPUCommands.h"

namespace Pudu
{

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
	VkRenderingInfo RenderPass::GetRenderingInfo(RenderFrameData& data)
	{
		VkRenderingInfo renderInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
		renderInfo.renderArea = renderArea;
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = attachments.colorAttachmentCount;
		renderInfo.pColorAttachments = attachments.colorAttachments;
		renderInfo.pDepthAttachment = attachments.depthAttachments;
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
	void ComputeRenderPass::BeginRender(RenderFrameData& data)
	{
	}
	void ComputeRenderPass::EndRender(RenderFrameData& data)
	{
	}
}

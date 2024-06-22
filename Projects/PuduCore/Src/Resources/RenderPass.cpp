#include "Resources/RenderPass.h"

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
		
		depthAttachments[depthAttachmentCount++] = attachmentInfo;

		return *this;
	}

	RenderPassAttachments& RenderPassAttachments::SetDepthStencilOperations(RenderPassOperation depth, RenderPassOperation stencil)
	{
		depthOperation = depth;
		stencilOperation = stencil;

		return *this;
	}
	VkRenderingInfo RenderPass::GetRenderingInfo()
	{
		VkRenderingInfo renderInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
		renderInfo.renderArea = renderArea;
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = attachments.colorAttachmentCount;
		renderInfo.pColorAttachments = attachments.colorAttachments;
		renderInfo.pDepthAttachment = attachments.depthAttachments;

		return renderInfo;
	}

	void RenderPass::SetName(const char* name)
	{
		this->name.append(name);
	}
}

#pragma once
#include <vulkan/vulkan_core.h>
#include "PuduConstants.h"
#include "Resources/Resources.h"
#include "Texture2D.h"

namespace Pudu
{
	class PuduGraphics;

	enum RenderPassOperation
	{
		DontCare,
		Load,
		Clear,
		Count
	}; // enum Enum


	struct RenderPassAttachment
	{
		SPtr<Texture2d> texture;
		VkAttachmentLoadOp loadOperation;
		VkAttachmentStoreOp storeOp;
		VkClearValue clearValue;
		VkImageLayout layout;

	};

	struct RenderPassAttachments
	{
		VkFormat depthStencilFormat;
		VkImageLayout depthStencilFinalLayout;

		RenderPassOperation depthOperation = RenderPassOperation::DontCare;
		RenderPassOperation stencilOperation = RenderPassOperation::DontCare;

		RenderPassAttachments& Reset();
		RenderPassAttachments& AddColorAttachment(RenderPassAttachment attachment);
		RenderPassAttachments& SetDepthStencilAttachment(RenderPassAttachment attachment);
		RenderPassAttachments& SetDepthStencilOperations(RenderPassOperation depth, RenderPassOperation stencil);

		uint16_t colorAttachmentCount = 0;
		uint16_t numColorFormats = 0;
		uint16_t depthAttachmentCount = 0;

		VkFormat colorAttachmentsFormat[K_MAX_IMAGE_OUTPUTS];
		VkRenderingAttachmentInfo colorAttachments[K_MAX_IMAGE_OUTPUTS];
		VkRenderingAttachmentInfo depthAttachments[K_MAX_IMAGE_OUTPUTS];
		VkRenderingAttachmentInfo stencilAttachments[K_MAX_IMAGE_OUTPUTS];
	};


	class RenderPass
	{
	public:
		RenderPass() = default;

		VkRenderingInfo GetRenderingInfo();
		VkRect2D renderArea;

		RenderPassHandle handle;
		VkRenderPass vkHandle;

		RenderPassAttachments attachments;
		uint16_t dispatchX = 0;
		uint16_t dispatchY = 0;
		uint16_t dispatchZ = 0;

		uint8_t numRenderTargets = 0;

		std::string name;

		void SetName(const char* name);
	};
}



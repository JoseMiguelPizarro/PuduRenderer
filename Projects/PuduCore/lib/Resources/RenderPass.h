#pragma once
#include <vulkan/vulkan_core.h>
#include "PuduConstants.h"
#include "Resources/Resources.h"
#include "Texture2D.h"
#include "RenderFrameData.h"

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

	struct RenderPassCreationData
	{
		bool isCompute;
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
		VkFormat GetStencilFormat();

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

		VkRenderingInfo GetRenderingInfo(RenderFrameData& data);
		VkRect2D renderArea;

		RenderPassHandle handle;
		VkRenderPass vkHandle;

		virtual void BeginRender(RenderFrameData& data);
		virtual void EndRender(RenderFrameData& data);

		RenderPassAttachments attachments;
		uint16_t dispatchX = 0;
		uint16_t dispatchY = 0;
		uint16_t dispatchZ = 0;

		uint8_t numRenderTargets = 0;

		std::string name;

		void SetName(const char* name);
	};

	class ComputeRenderPass :public RenderPass
	{
		void BeginRender(RenderFrameData& data) override;
		void EndRender(RenderFrameData& data) override;
	};
}



#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "Resources.h"

namespace Pudu
{
	struct RenderPassCreationData
	{
		uint16_t numRenderTargets = 0;
		VkFormat colorFormats[K_MAX_IMAGE_OUTPUTS];
		VkImageLayout colorFinalLayouts[K_MAX_IMAGE_OUTPUTS];
		RenderPassOperation colorOperations[K_MAX_IMAGE_OUTPUTS];

		VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;
		VkImageLayout depthStencilFinalLayout;

		RenderPassOperation depthOperation = RenderPassOperation::DontCare;
		RenderPassOperation stencilOperation = RenderPassOperation::DontCare;

		const char* name = nullptr;

		RenderPassCreationData& Reset();
		RenderPassCreationData& AddAttachment(VkFormat format, VkImageLayout layout, RenderPassOperation loadOp);
		RenderPassCreationData& SetDepthStencilTexture(VkFormat format, VkImageLayout layout);
		RenderPassCreationData& SetName(const char* name);
		RenderPassCreationData& SetDepthStencilOperation(RenderPassOperation depth, RenderPassOperation stencil);
	};
}


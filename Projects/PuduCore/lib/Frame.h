#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace Pudu
{
	struct Frame {
		VkCommandBuffer CommandBuffer;
		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;
		VkFence InFlightFence;
	};
}
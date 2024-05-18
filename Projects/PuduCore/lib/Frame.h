#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "GPUCommands.h"

namespace Pudu
{
	struct Frame {
		GPUCommands CommandBuffer;
		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;
		VkFence InFlightFence;
	};
}
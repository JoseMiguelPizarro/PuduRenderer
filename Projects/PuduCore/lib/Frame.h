#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "GPUCommands.h"

namespace Pudu
{
	struct Frame {
		GPUCommands CommandBuffer;
		GPUCommands ComputeCommandBuffer;
		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;
		VkFence InFlightFence;
	};
}
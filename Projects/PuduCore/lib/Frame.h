#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "GPUCommands.h"

namespace Pudu
{
	struct Frame {
		//TODO: ALLOCATE COMMANDS IN RESOURCE MANAGER AND USE POINTERS INSTEAD
		GPUCommands CommandBuffer;
		GPUCommands ComputeCommandBuffer;
		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;
		VkFence InFlightFence;
	};
}
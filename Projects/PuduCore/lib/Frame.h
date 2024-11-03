#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "GPUCommands.h"
#include "Semaphore.h"

namespace Pudu
{
	struct Frame {
		//TODO: ALLOCATE COMMANDS IN RESOURCE MANAGER AND USE POINTERS INSTEAD
		GPUCommands CommandBuffer;
		GPUCommands ComputeCommandBuffer;
		SPtr<Semaphore> ImageAvailableSemaphore;
		SPtr<Semaphore> RenderFinishedSemaphore;
		VkFence InFlightFence;
	};
}
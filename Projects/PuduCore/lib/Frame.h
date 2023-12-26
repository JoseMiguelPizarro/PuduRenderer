#pragma once
#include <vector>
#include <vulkan/vulkan.h>
struct Frame {
public:
	VkCommandBuffer CommandBuffer;
	VkSemaphore ImageAvailableSemaphore;
	VkSemaphore RenderFinishedSemaphore;
	VkFence InFlightFence;
};
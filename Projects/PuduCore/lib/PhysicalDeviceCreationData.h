#pragma once
#include "vulkan/vulkan_core.h"
struct PhysicalDeviceCreationData {
	VkPhysicalDeviceDescriptorIndexingFeatures IndexingFeatures;
	bool SupportsBindless;
	size_t PoolSizesCount;
};

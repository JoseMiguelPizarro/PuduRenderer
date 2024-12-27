#pragma once
#include "vulkan/vulkan_core.h"
struct PhysicalDeviceCreationData {
	VkPhysicalDeviceDescriptorIndexingFeatures IndexingFeatures;
	bool SupportsBindless;

	/// <summary>
	/// Number of resources pools
	/// </summary>
	size_t PoolSizesCount; 
	VkPhysicalDeviceFeatures2 features;
};

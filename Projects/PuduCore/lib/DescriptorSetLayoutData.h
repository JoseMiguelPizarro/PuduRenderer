#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vector>

namespace Pudu {
	struct DescriptorSetLayoutData {
		uint32_t SetNumber;
		VkDescriptorSetLayoutCreateInfo CreateInfo;
		std::vector<VkDescriptorSetLayoutBinding> Bindings;
	};

	struct DescriptorsCreationData {
		std::vector<DescriptorSetLayoutData> layoutData;
		uint16_t setsCount;
	};
}

#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vector>
#include "Resources/GPUResource.h"

namespace Pudu {

	struct DescriptorBinding
	{
		VkDescriptorType type;
		uint16_t index = 0;
		uint16_t count = 0;
		uint16_t set = 0;

		std::string name;
	}; // struct DescriptorBinding

	struct DescriptorSetLayout :GPUResource<DescriptorSetLayout>
	{
		VkDescriptorSetLayout vkHandle;

		VkDescriptorSetLayoutBinding* vkBinding = nullptr;
		DescriptorBinding* bindings = nullptr;
		uint8_t* indexToBinding = nullptr; // Mapping between binding point and binding data.
		uint16_t bindingsCount = 0;
		uint16_t setIndex = 0;
		uint8_t bindless = 0;
	}; // struct DesciptorSetLayoutVulkan

	struct DescriptorSetLayoutData {
		uint32_t SetNumber;
		VkDescriptorSetLayoutCreateInfo CreateInfo;
		std::vector<VkDescriptorSetLayoutBinding> Bindings;
	};

	struct DescriptorsCreationData {
		std::vector<DescriptorSetLayoutData> layoutData;
		uint16_t setsCount;
		std::vector<DescriptorBinding> bindingsData;
	};
}

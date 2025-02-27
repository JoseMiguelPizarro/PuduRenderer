#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vector>
#include "Resources/GPUResource.h"

namespace Pudu {

	struct DescriptorBinding
	{
		VkDescriptorType type;
		u16 index = 0;
		u16 count = 0;
		u16 set = 0;

		std::string name;
	}; // struct DescriptorBinding

	struct DescriptorSetLayout :GPUResource<DescriptorSetLayout>
	{
		std::string name;
		VkDescriptorSetLayout vkHandle;
		VkDescriptorSetLayoutBinding* vkBinding = nullptr;
		DescriptorBinding* bindings = nullptr;
		u8* indexToBinding = nullptr; // Mapping between binding point and binding data.
		u16 bindingsCount = 0;
		u16 setIndex = 0;
		u8 bindless = 0;
	}; // struct DesciptorSetLayoutVulkan

	struct DescriptorSetLayoutData {
		std::string name;
		u32 SetNumber;
		VkDescriptorSetLayoutCreateInfo CreateInfo;
		std::vector<VkDescriptorSetLayoutBinding> Bindings;
		bool bindless;
	};

	struct DescriptorSetLayoutsData {
		std::vector<DescriptorSetLayoutData> layoutData;
		u16 setsCount;
		std::vector<DescriptorBinding> bindingsData;
	};
}

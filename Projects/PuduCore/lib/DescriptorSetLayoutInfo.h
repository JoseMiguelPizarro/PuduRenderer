#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vector>
#include "Resources/GPUResource.h"
#include "Resources/ConstantBufferInfo.h"
namespace Pudu {

	//Represents a binding
	struct DescriptorBinding
	{
		VkDescriptorType type;
		u16 index = 0;
		u16 count = 0;
		u16 setNumber = 0;

		std::string name;
		ConstantBufferInfo cbInfo;

		VkDescriptorSetLayoutBinding ToVKDescriptorSetLayoutBinding() const
		{
			VkDescriptorSetLayoutBinding binding;
			binding.binding = index;
			binding.descriptorType = type;
			binding.descriptorCount = count;
			binding.stageFlags =VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT |
					VK_SHADER_STAGE_COMPUTE_BIT; //TODO: Hack, but in all stages for now
			binding.pImmutableSamplers = nullptr;

			return binding;
		}
	}; // struct DescriptorBinding

	//Represents an allocated descriptor set layout
	struct DescriptorSetLayout :GPUResource<DescriptorSetLayout>
	{
		std::string name;
		std::string scope;
		VkDescriptorSetLayout vkHandle;
		VkDescriptorSetLayoutBinding* vkBinding = nullptr;
		DescriptorBinding* bindings = nullptr;
		u8* indexToBinding = nullptr; // Mapping between binding point and binding data.
		u16 bindingsCount = 0;
		u16 setIndex = 0;
		u8 bindless = 0;
	}; // struct DesciptorSetLayoutVulkan

//Info used to create a DescriptorSetLayout
	struct DescriptorSetLayoutInfo {
		std::string name;
		u32 SetNumber;
		VkDescriptorSetLayoutCreateInfo CreateInfo;
		std::vector<VkDescriptorSetLayoutBinding> Bindings;
		std::string scope;
		bool bindless;
	};
}

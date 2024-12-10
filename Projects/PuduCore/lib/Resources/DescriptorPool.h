#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include "GPUResource.h"
namespace Pudu {

	struct DescriptorPoolCreationData {
		bool bindless;
		std::vector<VkDescriptorPoolSize> poolSizes;

		DescriptorPoolCreationData* AddDescriptorType(VkDescriptorType type, uint32_t count);
	};


	class DescriptorPool :public GPUResource<DescriptorPool>
	{
	public:
		VkDescriptorPool vkHandle;
	};

}

#pragma once
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <DescriptorSetLayoutData.h>
#include "Resources/Resources.h"

namespace Pudu {

	class SPIRVParser
	{
	public:
		static void GetDescriptorSetLayout(const char* spirvData, uint32_t size, std::vector<DescriptorSetLayoutData>& outDescriptorSetLayoutData);
		static void GetDescriptorSetLayout(PipelineCreationData& creationData, std::vector<DescriptorSetLayoutData>& outDescriptorSetLayoutData);
	};
}



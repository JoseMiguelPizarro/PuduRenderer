#pragma once
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <DescriptorSetLayoutInfo.h>
#include "Resources/Resources.h"
#include "Shader.h"

namespace Pudu {

	class SPIRVParser
	{
	public:
		static void GetDescriptorSetLayout(const char* entryPoint, const void* spirvData, uint32_t size, DescriptorSetLayoutsCollection& outDescriptorSetLayoutData);
		static void GetDescriptorSetLayout(Shader* creationData, DescriptorSetLayoutsCollection& outDescriptorSetLayoutData);
	};
}



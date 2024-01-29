#pragma once
#include <vector>
#include "DescriptorSetLayoutData.h"

namespace Pudu {
	struct PipelineCreationData
	{
		std::vector<char> vertexShaderData;
		std::vector<char> fragmentShaderData;
		std::vector<DescriptorSetLayoutData> descriptorSetLayoutData;
	};
}


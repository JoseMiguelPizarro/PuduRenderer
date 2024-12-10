#include "Resources/DescriptorPool.h"


namespace Pudu {

	DescriptorPoolCreationData* DescriptorPoolCreationData::AddDescriptorType(VkDescriptorType type, uint32_t count)
	{
		poolSizes.push_back({ type,count });

		return this;
	}
}

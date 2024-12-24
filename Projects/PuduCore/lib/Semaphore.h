#pragma once
#include "Resources/GPUResource.h"
#include "Resources/Resources.h"
#include <vulkan/vulkan_core.h>
namespace Pudu
{
	struct Semaphore :GPUResource<Semaphore>
	{
		VkSemaphore vkHandle;

		operator VkSemaphore& () { return vkHandle; }
		operator VkSemaphore* () { return &vkHandle; }
		operator VkSemaphore() const { return vkHandle; }


		uint64_t Signal();
		uint64_t TimelineValue();

	private:
		uint64_t m_timelineValue;
	};
}


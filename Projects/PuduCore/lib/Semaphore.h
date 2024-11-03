#pragma once
#include "Resources/Resources.h"
#include <vulkan/vulkan_core.h>
namespace Pudu
{
	struct Semaphore
	{
		SemaphoreHandle handle;
		VkSemaphore vkHandle;

		operator VkSemaphore& () { return vkHandle; }
		operator VkSemaphore* () { return &vkHandle; }
		operator VkSemaphore() const { return vkHandle; }
	};
}


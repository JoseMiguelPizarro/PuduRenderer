#pragma once
#include "GPUResource.h"
namespace Pudu
{
	class CommandPool :public GPUResource<CommandPool>
	{
	public:
		VkCommandPool vkHandle;
	};
}


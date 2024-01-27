#pragma once

#include "PuduCore.h"
#include "TextureSampler.h"
#include <vulkan/vulkan_core.h>
#include <string>

namespace Pudu {
	class Texture2d
	{
	public:
		VkImage vkImageHandler;
		VkDeviceMemory vkMemoryHandler;
		VkImageView vkImageViewHandler;
		TextureSampler Sampler;
		Handle Handler;

		void Dispose();

	private:
		bool m_disposed;
	};
}
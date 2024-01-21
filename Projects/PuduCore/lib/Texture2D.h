#pragma once

#include <vulkan/vulkan_core.h>
#include <string>

namespace Pudu {
	class Texture2d
	{
	public:
		VkImage ImageHandler;
		VkDeviceMemory MemoryHandler;
		VkImageView ImageViewHandler;

		void Dispose();

	private:
		bool m_disposed;
	};
}
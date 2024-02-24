#pragma once

#include <string>
#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>

#include "Resources/Resources.h"
#include "PuduCore.h"
#include "TextureSampler.h"

namespace Pudu {
	class Texture2d
	{
	public:
		char const* name;
		VkImage vkImageHandle;
		VkDeviceMemory vkMemoryHandle;
		VkImageView vkImageViewHandle;
		VmaAllocation vmaAllocation;
		VkImageLayout vkImageLayout;
		TextureSampler Sampler;
		TextureHandle handle;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		VkFormat format;

		void Dispose();

	private:
		bool m_disposed;
	};
}
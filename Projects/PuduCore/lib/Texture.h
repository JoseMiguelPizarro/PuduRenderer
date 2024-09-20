#pragma once

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
#include "Resources/Resources.h"
#include "TextureSampler.h"

namespace Pudu
{
	class Texture
	{
	public:
		std::string name;
		VkImage vkImageHandle;
		VkImageView vkImageViewHandle;
		VkDeviceMemory vkMemoryHandle;
		VmaAllocation vmaAllocation;
		VkImageLayout vkImageLayout;
		TextureSampler Sampler;
		TextureHandle handle;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		VkFormat format;

		void Dispose();

		//Virtual to make this a polymorphic class
		virtual ~Texture() = default;

	private:
		bool m_disposed;
	};
}

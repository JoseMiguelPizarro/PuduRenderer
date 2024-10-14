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
		bool isSwapChain = false;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		uint32_t mipLevels = 1;
		VkFormat format;
		uint32_t layers = 1;

		uint32_t size;

		bool IsDestroyed();

		//Virtual to make this a polymorphic class
		virtual ~Texture() = default;

	private:
		friend PuduGraphics;
		void Destroy();
		bool m_disposed;
	};
}

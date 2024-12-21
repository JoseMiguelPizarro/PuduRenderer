#pragma once

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
#include "Resources/Resources.h"
#include "TextureSampler.h"
#include "ShaderObject.h"

namespace Pudu
{
	class Texture : public GPUResource<Texture>
	{
	public:
		VkImage vkImageHandle;
		VkImageView vkImageViewHandle;
		VkDeviceMemory vkMemoryHandle;
		VmaAllocation vmaAllocation;
		VkImageLayout vkImageLayout;
		TextureSampler Sampler;
		VkImageLayout layout;

		bool isSwapChain = false;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		uint32_t mipLevels = 1;
		VkFormat format;
		uint32_t layers = 1;
		bool bindless;
		void* sourceData;
		void* pixels;
		uint32_t dataSize;

		bool IsDestroyed();

		TextureFlags::Enum GetFlags() { return m_flags; }

		GPUResourceType::Type Type() { return GPUResourceType::Texture; }
		virtual TextureType::Enum GetTextureType() { return TextureType::Texture2D; }

		Texture() {
			m_flags = PopulateFlags();
		}


	protected:
		virtual TextureFlags::Enum PopulateFlags() { return TextureFlags::Default; };

	private:
		friend PuduGraphics;
		void Destroy();

		void OnCreate(PuduGraphics* gfx) override;
		bool m_disposed;
		TextureFlags::Enum m_flags = TextureFlags::Default;
	};


	class RenderTexture : public Texture
	{
	protected:
		TextureFlags::Enum PopulateFlags() override { return TextureFlags::RenderTarget; };
	};
}

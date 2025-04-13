#pragma once

#include <boolinq.h>
#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
#include "Resources/Resources.h"
#include "TextureSampler.h"
#include "Enums/ResourceUsage.h"

namespace Pudu
{
    class Texture : public GPUResource<Texture>
    {
    public:
        VkImage vkImageHandle;
        VkImageView vkImageViewHandle;
        VkDeviceMemory vkMemoryHandle;
        VmaAllocation vmaAllocation;
        TextureSampler Sampler;

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
        void* samplerData;
        uint32_t dataSize;

        bool IsDestroyed();

        TextureFlags::Enum GetFlags() { return m_flags; }

        GPUResourceType::Type Type() { return GPUResourceType::Texture; }
        virtual TextureType::Enum GetTextureType() { return TextureType::Texture2D; }
        void SetImageLayout(VkImageLayout layout);
        VkSampleCountFlagBits GetSampleCount() const { return m_sampleCount; }
        void SetSampleCount(VkSampleCountFlagBits count) { m_sampleCount = count; };
        VkImageLayout GetImageLayout() const;


        Texture()
        {
            m_flags = PopulateFlags();
        }

    protected:
        virtual TextureFlags::Enum PopulateFlags() { return TextureFlags::Default; };

    private:
        friend PuduGraphics;
        void Destroy();
        void OnCreate(PuduGraphics* gfx) override;

        VkSampleCountFlagBits m_sampleCount = VK_SAMPLE_COUNT_1_BIT;
        bool m_disposed;
        VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        TextureFlags::Enum m_flags = TextureFlags::Default;
    };


    class RenderTexture : public Texture
    {
    public:
        void SetUsage(ResourceUsage usage);


        ResourceUsage GetUsage() const;

    protected:
        TextureFlags::Enum PopulateFlags() override { return TextureFlags::RenderTarget; };
        ResourceUsage m_usage = ResourceUsage::UNDEFINED;
        u32 m_sampleCount = 1;
    };
}

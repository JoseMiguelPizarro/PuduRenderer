#include "Texture.h"
#include "PuduGraphics.h"

namespace Pudu
{
    void Texture::Destroy()
    {
        if (!m_disposed)
        {
            m_disposed = true;
        }
        else
        {
            LOG("Trying to dispose an already disposed texture {}", name);
        }
    }

    void Texture::OnCreate(PuduGraphics* gfx)
    {
        SamplerCreationData* samplerPtr = static_cast<SamplerCreationData*>(samplerData);

        SamplerCreationData sampler;
        sampler.wrap = false;
        sampler.maxLOD = mipLevels - 1;


        if (samplerData == nullptr)
        {
            samplerPtr = &sampler;
        }

        gfx->CreateVKTexture(this);
        gfx->CreateVKTextureSampler(*samplerPtr, Sampler.vkHandle);
    }

    void RenderTexture::SetUsage(const ResourceUsage usage)
    {
        m_usage = usage;
    }

    void RenderTexture::OnCreate(PuduGraphics* gfx)
    {
        Texture::OnCreate(gfx);

        if (m_usage == ResourceUsage::UNORDERED_ACCESS)
        {
            auto cmd = gfx->BeginSingleTimeCommands();
            cmd.TransitionTextureLayout(this, VK_IMAGE_LAYOUT_GENERAL);
            gfx->EndSingleTimeCommands(cmd);
        }
    }

    ResourceUsage RenderTexture::GetUsage() const
    {
        return m_usage;
    }

    bool Texture::IsDestroyed()
    {
        return m_disposed;
    }

    void Texture::SetImageLayout(VkImageLayout layout)
    {
        this->m_layout = layout;
    }

    VkImageLayout Texture::GetImageLayout() const
    {
        return m_layout;
    }
}

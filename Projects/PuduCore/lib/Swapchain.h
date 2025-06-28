//
// Created by Hojaverde on 6/28/2025.
//

#pragma once
#include "vulkan/vulkan_core.h"
#include "Texture.h"

namespace Pudu
{
    struct Swapchain
    {
        VkImage images[4];
        SPtr<RenderTexture> textures[4];
        VkImageView imageViews[4];
        VkSwapchainKHR swapchainHandle;
        Size imageCount;

        VkImage GetImage(uint32_t index);
        VkImageView GetImageView(uint32_t index);
    };
};

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
        VkImage images[k_MAX_SWAP_CHAIN_IMAGES];
        SPtr<RenderTexture> textures[k_MAX_SWAP_CHAIN_IMAGES];
        SPtr<Semaphore> renderCompleteSemaphores[k_MAX_SWAP_CHAIN_IMAGES];

        VkImageView imageViews[k_MAX_SWAP_CHAIN_IMAGES];
        VkSwapchainKHR swapchainHandle;
        Size imageCount;


        VkImage GetImage(uint32_t index);
        VkImageView GetImageView(uint32_t index);
    };
};

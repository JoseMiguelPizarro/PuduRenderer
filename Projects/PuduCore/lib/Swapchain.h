//
// Created by Hojaverde on 6/28/2025.
//

#pragma once
#include "vulkan/vulkan_core.h"

namespace Pudu
{
     struct Swapchain
     {

       VkImage images[4];
       VkImageView imageViews[4];
       VkSwapchainKHR swapchainHandle;


       VkImage GetImage(uint32_t index);
       VkImageView GetImageView(uint32_t index);
     };
};
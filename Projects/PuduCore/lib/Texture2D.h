#pragma once

#include <vulkan/vulkan_core.h>
#include <string>

class Texture2d
{
public:
    VkImage ImageHandler;
    VkDeviceMemory MemoryHandler;
    VkImageView ImageViewHandler;
};

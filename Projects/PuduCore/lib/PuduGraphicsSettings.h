#pragma once
#include "GPUEnums.h"
#include "Model.h"
#include "PuduCore.h"

namespace Pudu
{
struct PuduGraphicsSettings
{
    int2 resolution;
    PresentMode presentMode = PresentMode::FIFO;
    VkColorSpaceKHR surfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR; // Tells how the display engine should interpret the framebuffer value
    VkFormat surfaceFormat = VK_FORMAT_B8G8R8A8_UNORM; //If SRGB format, this forces image writen to swapchain to be converted into srgb, otherwise gamma correction should be done manually before presenting
};

}

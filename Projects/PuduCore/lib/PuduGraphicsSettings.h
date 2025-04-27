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
    VkColorSpaceKHR surfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    VkFormat surfaceFormat = VK_FORMAT_B8G8R8A8_UNORM; //Apparently default swapchain format on windows
};

}

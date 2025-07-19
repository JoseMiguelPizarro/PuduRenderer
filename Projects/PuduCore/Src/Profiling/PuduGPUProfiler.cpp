//
// Created by Hojaverde on 7/19/2025.
//


#include <vulkan/vulkan_core.h>
#include "Profiling/PuduGPUProfiler.h"

namespace Pudu
{
    GPUProfiler::~GPUProfiler()
    {
        TracyVkDestroy(m_context);
    }

    void GPUProfiler::Init(VkPhysicalDevice physicalDevice, VkDevice device,
                           PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT gpdctd,
                           PFN_vkGetCalibratedTimestampsEXT gct)
    {
        m_context = TracyVkContextHostCalibrated(
            physicalDevice, device, vkResetQueryPool,
            gpdctd,
            gct);
    }
}




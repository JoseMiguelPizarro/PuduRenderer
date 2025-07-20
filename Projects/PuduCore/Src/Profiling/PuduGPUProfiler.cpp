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

    void GPUProfiler::Init(VkPhysicalDevice physicalDevice, VkDevice device,PFN_vkResetQueryPool rqp,
                           PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT gpdctd,
                           PFN_vkGetCalibratedTimestampsEXT gct)
    {
        m_context = TracyVkContextHostCalibrated(
            physicalDevice, device, rqp,
            gpdctd,
            gct);
    }

    void GPUProfiler::Enable()
    {
        m_enabled = true;
    }

    void GPUProfiler::Disable()
    {
        m_enabled = false;
    }
}




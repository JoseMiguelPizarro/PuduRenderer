//
// Created by Hojaverde on 7/19/2025.
//

#pragma once
#include <vulkan/vulkan_core.h>
#include <tracy/TracyVulkan.hpp>

namespace Pudu
{
    class GPUProfiler
    {
    public:
        GPUProfiler() = default;
        ~GPUProfiler();
        TracyVkCtx GetContext() const { return m_context; }

        void Init(VkPhysicalDevice physicalDevice, VkDevice device,
                  PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT gpdctd, PFN_vkGetCalibratedTimestampsEXT
                  gct);

    private:
        friend class PuduGraphics;
        TracyVkCtx m_context = nullptr;
    };
}


#ifdef GPU_PROFILE
#define PROFILE_GPU_FUNCTION() ZoneScoped
#define PROFILE_GPU_ZONE(profiler, name,cmdBuffer) TracyVkZone(profiler->GetContext(), cmdBuffer, name);
#else
#define PROFILE_GPU_FUNCTION()
#define PROFILE_GPU_ZONE(profiler, name,cmdBuffer)
#endif

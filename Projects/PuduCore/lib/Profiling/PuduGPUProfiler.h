//
// Created by Hojaverde on 7/19/2025.
//

#pragma once
#include <vulkan/vulkan_core.h>
#ifdef GPU_PROFILE
#include <tracy/TracyVulkan.hpp>
#endif

namespace Pudu
{
    class GPUProfiler
    {
    public:
        GPUProfiler() = default;
        ~GPUProfiler();
        void* GetContext() const { return m_context; }

        void Init(VkPhysicalDevice physicalDevice, VkDevice device,
                  PFN_vkResetQueryPool rqp,
                  PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT gpdctd, PFN_vkGetCalibratedTimestampsEXT
                  gct);

        void Enable();
        void Disable();
        bool IsEnabled() const { return m_enabled; }

    private:
        friend class PuduGraphics;
        void* m_context = nullptr;
        bool m_enabled = false;
    };
}

#ifdef GPU_PROFILE
#define ToTracyCTX(profilerContext) static_cast<tracy::VkCtx*>(profilerContext)
#define PROFILE_GPU_FUNCTION() ZoneScoped
#define PROFILE_GPU_ZONE(profiler, name,cmdBuffer)  TracyVkZone(ToTracyCTX(profiler->GetContext()), cmdBuffer, name);
#define PROFILE_GPU_COLLECT(profiler, cmdBuffer) TracyVkCollect(ToTracyCTX(profiler->GetContext()), cmdBuffer);
#define PROFILER_ZONE(name, color) \
{                                    \
ZoneScopedC(color);                \
ZoneName(name, strlen(name))
#define PROFILER_ZONE_END() }
#else
#define PROFILE_GPU_FUNCTION()
#define PROFILE_GPU_ZONE(profiler, name,cmdBuffer)
#define PROFILE_GPU_COLLECT(profiler)
#define PROFILER_ZONE(name, color)
#define PROFILER_ZONE_END()
#endif

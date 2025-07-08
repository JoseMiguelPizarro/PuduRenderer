#pragma once
#include "GPUResource.h"
#include "vulkan/vulkan_core.h"

namespace Pudu
{
    class CommandPool : public GPUResource<CommandPool>
    {
    public:
        GPUResourceType Type() override { return GPUResourceType::CommandPool; }

        VkCommandPool vkHandle;
    };
}

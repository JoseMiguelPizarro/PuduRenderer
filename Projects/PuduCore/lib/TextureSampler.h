#pragma once
#include "vulkan/vulkan_core.h"
#include "Resources/GPUResource.h"
namespace Pudu
{
    struct TextureSampler : GPUResource<TextureSampler>
    {
        GPUResourceType Type() override {return GPUResourceType::Sampler;}
        VkSampler vkHandle;
    };
}

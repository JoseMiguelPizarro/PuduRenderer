#pragma once

#include "DescriptorSetLayoutInfo.h"
#include "Logger.h"
#include "ShaderCompilation/ShaderLayout.h"

namespace Pudu
{
    class IDescriptorProvider
    {
    public:
        virtual ~IDescriptorProvider() = default;

        virtual DescriptorBinding* GetBindingByName(const char* name)
        {
            LOG_ERROR("Not implemented");
            return nullptr;
        };

        virtual std::vector<SPtr<DescriptorSetLayout>>* GetDescriptorSetLayouts()
        {
            LOG_ERROR("Not implemented");
            return nullptr;
        };

        virtual VkDescriptorSetLayout* GetVkDescriptorSetLayouts()
        {
            LOG_ERROR("Not implemented");
            return nullptr;
        };

        virtual ShaderNode* GetShaderLayout()
        {
            LOG_ERROR("Not implemented");
            return nullptr;
        };
    };
}

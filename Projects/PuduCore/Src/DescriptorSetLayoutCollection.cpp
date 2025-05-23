//
// Created by Hojaverde on 3/9/2025.
//
#include <algorithm>
#include <ranges>
#include "DescriptorSetLayoutCollection.h"

#include "Logger.h"

namespace Pudu
{
    DescriptorBinding* DescriptorSetLayoutsCollection::GetBindingByName(const char* name)
    {
        for (auto& binding : bindingsData)
        {
            if (strcmp(binding.name.c_str(), name) == 0)
                return &binding;
        }
        // const auto v = (std::ranges::find_if(bindingsData, [&name](const DescriptorBinding& binding)
        // {
        //     return !strcmp(name, binding.name.c_str());
        // }));
        //
        // if (v == bindingsData.end())
        //     return nullptr;
        //
        // return v.operator->();

        return nullptr;
    }

    std::vector<SPtr<DescriptorSetLayout>>* DescriptorSetLayoutsCollection::GetDescriptorSetLayouts()
    {
        ASSERT(m_descriptorSetLayoutsCreated, "Trying to get unallocated DescriptorSetLayouts");
        return &m_setLayouts;
    }

    VkDescriptorSetLayout* DescriptorSetLayoutsCollection::GetVkDescriptorSetLayouts()
    {
        return m_vkSetLayouts.data();
    }

    ShaderNode* DescriptorSetLayoutsCollection::GetShaderLayout()
    {
        return m_shaderLayout.get();
    }
}

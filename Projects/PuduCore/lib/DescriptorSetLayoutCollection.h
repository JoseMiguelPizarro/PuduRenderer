//
// Created by Hojaverde on 3/9/2025.
//
#pragma once

#include "DescriptorSetLayoutInfo.h"
#include "IDescriptorProvider.h"

namespace Pudu
{
    //Collection of Descriptor layout info and their bindings
    struct DescriptorSetLayoutsCollection : IDescriptorProvider
    {
        u16 setsCount;
        std::vector<DescriptorSetLayoutInfo> setLayoutInfos;
        std::vector<DescriptorBinding> bindingsData;

        //IDescriptorProvider
        DescriptorBinding* GetBindingByName(const char* name) override;
        std::vector<SPtr<DescriptorSetLayout>>* GetDescriptorSetLayouts() override;
        VkDescriptorSetLayout* GetVkDescriptorSetLayouts() override;

    private:
        friend class PuduGraphics;
        std::vector<SPtr<DescriptorSetLayout>> m_setLayouts;
        std::vector<VkDescriptorSetLayout> m_vkSetLayouts;

        bool m_descriptorSetLayoutsCreated = false;
    };

}

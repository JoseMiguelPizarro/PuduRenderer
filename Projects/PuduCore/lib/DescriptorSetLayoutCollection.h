//
// Created by Hojaverde on 3/9/2025.
//
#pragma once

#include "DescriptorSetLayoutInfo.h"
#include "IDescriptorProvider.h"
#include "ShaderCompilation/ShaderLayout.h"

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
        ShaderNode* GetShaderLayout();

    private:
        friend class PuduGraphics;
        friend class ShaderObjectLayoutBuilder;

        std::vector<SPtr<DescriptorSetLayout>> m_setLayouts;
        std::vector<VkDescriptorSetLayout> m_vkSetLayouts;
        ShaderNode m_shaderLayout;

        bool m_descriptorSetLayoutsCreated = false;
    };

}

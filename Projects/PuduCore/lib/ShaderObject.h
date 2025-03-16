#pragma once

#include "Texture.h"
#include "Resources/GPUResource.h"
#include "DescriptorSetLayoutInfo.h"
#include "IDescriptorProvider.h"
#include "DescriptorSetLayoutCollection.h"

namespace Pudu
{
    class Pipeline;
    class RenderPass;

    class IShaderObject : public IDescriptorProvider
    {
    public:
        virtual DescriptorSetLayoutsCollection* GetDescriptorSetLayoutsData() { return &m_descriptorLayoutsData; };
        virtual SPtr<Pipeline> CreatePipeline(PuduGraphics* graphics, RenderPass* renderPass) = 0;
        virtual VkShaderModule GetModule() { return m_module; }
        virtual void SetName(const char* name) = 0;
        virtual const char* GetName() = 0;
        u32 GetActiveLayoutCount() const { return numActiveLayouts; }

#pragma region DescriptorProvider
        std::vector<SPtr<DescriptorSetLayout>>* GetDescriptorSetLayouts() override { return &descriptorSetLayouts; };
        DescriptorBinding* GetBindingByName(const char* name) override;
        ShaderNode* GetShaderLayout() override;
        VkDescriptorSetLayout* GetVkDescriptorSetLayouts() override { return m_VkDescriptorSetLayouts.data(); }
#pragma endregion

    protected:
        friend class PuduGraphics;

        void SetDescriptorSetLayouts(const std::vector<SPtr<DescriptorSetLayout>>& layouts)
        {
            descriptorSetLayouts = layouts;

            for (const auto& layout : descriptorSetLayouts)
            {
                m_VkDescriptorSetLayouts.push_back(layout->vkHandle);
            }

            numActiveLayouts = descriptorSetLayouts.size();
        }

        VkShaderModule m_module;
        DescriptorSetLayoutsCollection m_descriptorLayoutsData;
        std::vector<SPtr<DescriptorSetLayout>> descriptorSetLayouts;
        std::vector<GPUResourceHandle<DescriptorSetLayout>> m_descriptorSetLayoutHandles;
        std::vector<VkDescriptorSetLayout> m_VkDescriptorSetLayouts;

        u32 numActiveLayouts;
    };
}

#pragma once

#include "Texture.h"
#include "Resources/GPUResource.h"
#include "DescriptorSetLayoutInfo.h"
#include "IDescriptorProvider.h"
#include "DescriptorSetLayoutCollection.h"
#include "ShaderCompilation/ShaderCompilationObject.h"

namespace Pudu
{
    class Pipeline;
    class RenderPass;

    class IShaderObject : public IDescriptorProvider
    {
    public:
        virtual DescriptorSetLayoutsCollection* GetDescriptorSetLayoutsData()
        {
            return &m_compilationObject.descriptorsData;
        };
        SPtr<Pipeline> CreatePipeline(PuduGraphics* graphics, RenderPass* renderPass);
        virtual VkShaderModule GetModule() { return m_module; }
        virtual void SetName(const char* name) = 0;
        virtual const char* GetName() = 0;
        u32 GetActiveLayoutCount() const { return numActiveLayouts; }
        bool NeedsPipelineRebuild() { return m_needsPipelineRebuild; }
        std::vector<GPUResourceHandle<Pipeline>>* GetPipelines() { return &m_pipelines; }


#pragma region DescriptorProvider
        std::vector<SPtr<DescriptorSetLayout>>* GetDescriptorSetLayouts() override { return &descriptorSetLayouts; };
        ShaderNode* GetShaderLayout() override;
        VkDescriptorSetLayout* GetVkDescriptorSetLayouts() override { return m_VkDescriptorSetLayouts.data(); }
#pragma endregion

    protected:
        friend class PuduGraphics;
        virtual SPtr<Pipeline> OnCreatePipeline(PuduGraphics* gfx, RenderPass* renderPass) =0;


        void SetCompilationObject(const ShaderCompilationObject& compilationObject)
        {
            m_compilationObject = compilationObject;
        };

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
        ShaderCompilationObject m_compilationObject;
        std::vector<SPtr<DescriptorSetLayout>> descriptorSetLayouts;
        std::vector<GPUResourceHandle<DescriptorSetLayout>> m_descriptorSetLayoutHandles;
        std::vector<VkDescriptorSetLayout> m_VkDescriptorSetLayouts;
        std::vector<GPUResourceHandle<Pipeline>> m_pipelines;
        fs::path m_shaderPath;
        bool m_needsPipelineRebuild = false;

        u32 numActiveLayouts;
    };
}

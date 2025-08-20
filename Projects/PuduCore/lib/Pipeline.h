#pragma once

#include "Resources/GPUResource.h"
#include "Resources/Resources.h"
#include "VertexLayout.h"

namespace Pudu
{
    class IShaderObject;
    class RenderPass;
    class Renderer;

    struct PipelineQueryData
    {
        RenderPass* renderPass;
        IShaderObject* shader;
        Renderer* renderer;
    };

    namespace PipelineType
    {
        enum Type
        {
            Graphics,
            Compute
        };
    };


    struct PipelineCreationData
    {
        const u32 *vertexShaderData;
        const u32 *fragmentShaderData;

        RasterizationCreation rasterization;
        DepthStencilCreation depthStencil;
        BlendStateCreation blendState;
        VertexInputCreation vertexInput;
        ShaderStateCreationData shadersStateCreationData;

        GPUResourceHandle<RenderPass> renderPassHandle;

        DescriptorSetLayoutsCollection descriptorCreationData;
        std::vector<SPtr<DescriptorSetLayout>> *descriptorSetLayouts;
        PushConstantInfo *pushConstants;

        VkDescriptorSetLayout *vkDescriptorSetLayout;
        u32 activeLayouts;
        bool multiSampled;

        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        const char *name = nullptr;
    };

    class Pipeline : public GPUResource<Pipeline>
    {
    public:
        GPUResourceType Type() override { return GPUResourceType::Pipeline; };

        std::string name;
        VkPipeline vkHandle;
        VkPipelineLayout vkPipelineLayoutHandle;

        VkPipelineBindPoint vkPipelineBindPoint;
        VkDescriptorSet vkDescriptorSets[K_MAX_DESCRIPTOR_SET_LAYOUTS]; //Just 1 for now, bindless

        uint32_t numDescriptorSets = 0;

        VkFormat depthStencilFormat;

        GPUResourceHandle<ShaderState> shaderState;

        uint32_t numActiveLayouts = 0;

        DepthStencilCreation depthStencil;
        BlendStateCreation blendState;
        RasterizationCreation rasterization;

        bool bindlessUpdated;
        PipelineType::Type pipelineType;

        GPUResourceHandle<RenderPass> m_renderPass;
        GPUResourceHandle<IShaderObject> m_shader;

        bool graphicsPipeline = true;

        //Set when the pipeline needs to be rebuild due to hot-reloading. The renderer can use this flag for its renderpass/shader cache
        bool isDirty = true;
    }; // struct Pipeline
}

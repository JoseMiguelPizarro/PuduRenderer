#include <vector>
#include <ranges>
#include <algorithm>
#include <vulkan/vulkan_core.h>
#include "ShaderCompiler/DescriptorsBuilder.h"
#include "PuduCore.h"
#include <slang-com-ptr.h>

#include "slang.h"
#include "ShaderCompilation/ShaderCompiler.h"

typedef SlangResult Result;

using Slang::ComPtr;

namespace Pudu
{
    VkDescriptorType MapSlangBindingTypeToVulkanDescriptorType(slang::BindingType bindingType)
    {
        switch (bindingType)
        {
#define CASE(FROM, TO)             \
case slang::BindingType::FROM: \
return VK_DESCRIPTOR_TYPE_##TO

        CASE(Sampler, SAMPLER);
        CASE(CombinedTextureSampler, COMBINED_IMAGE_SAMPLER);
        CASE(Texture, SAMPLED_IMAGE);
        CASE(MutableTexture, STORAGE_IMAGE);
        CASE(TypedBuffer, UNIFORM_TEXEL_BUFFER);
        CASE(MutableTypedBuffer, STORAGE_TEXEL_BUFFER);
        CASE(ConstantBuffer, UNIFORM_BUFFER);
        CASE(RawBuffer, STORAGE_BUFFER);
        CASE(MutableRawBuffer, STORAGE_BUFFER);
        CASE(InputRenderTarget, INPUT_ATTACHMENT);
        CASE(InlineUniformData, INLINE_UNIFORM_BLOCK);
        CASE(RayTracingAccelerationStructure, ACCELERATION_STRUCTURE_KHR);

#undef CASE

        default:
            return VkDescriptorType(-1);
        }
    }

    struct PipelineLayoutBuilder
    {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::vector<VkPushConstantRange> pushConstantRanges;
    };

    // What Goes Into a Descriptor Set Layout?
    // =======================================

    struct DescriptorSetLayoutBuilder
    {
        std::vector<VkDescriptorSetLayoutBinding> descriptorRanges;
        u32 setIndex = -1;
    };

    void FinisBuildingDescriptorSetLayout(PipelineLayoutBuilder& pipelineLayoutBuilder,
                                          DescriptorSetLayoutBuilder& descriptorSetLayoutBuilder)
    {
        if (descriptorSetLayoutBuilder.descriptorRanges.empty())
        {
            return;
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

        descriptorSetLayoutInfo.bindingCount = descriptorSetLayoutBuilder.descriptorRanges.size();
        descriptorSetLayoutInfo.pBindings = descriptorSetLayoutBuilder.descriptorRanges.data();


        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;


        //TODO: HERE IS WHERE WE CREATE THE DESCRIPTOR SET LAYOUTS
    }

    //Ordering of nested parameter blocks
    void StartBuildingDescriptorSetLayout(PipelineLayoutBuilder& pipelineLayoutBuilder,
                                          DescriptorSetLayoutBuilder& descriptorSetLayoutBuilder)
    {
        descriptorSetLayoutBuilder.setIndex = pipelineLayoutBuilder.descriptorSetLayouts.size();
        pipelineLayoutBuilder.descriptorSetLayouts.push_back(VK_NULL_HANDLE);
    }

    //Remove VK_NULL_HANDLE
    void FilterOutEmptyDescriptorSets(PipelineLayoutBuilder& pipelineLayoutBuilder)
    {
        std::vector<VkDescriptorSetLayout> fiilteredDescriptorSetLayouts;
        for (auto descriptorSetLayout : pipelineLayoutBuilder.descriptorSetLayouts)
        {
            if (!descriptorSetLayout)
                continue;
            fiilteredDescriptorSetLayouts.push_back(descriptorSetLayout);
        }

        std::swap(pipelineLayoutBuilder.descriptorSetLayouts, fiilteredDescriptorSetLayouts);
    }

    void AddDescriptorRange(DescriptorSetLayoutBuilder& descriptorSetLayoutBuilder,
                            slang::TypeLayoutReflection* typeLayout, u32 relativeSetIndex,
                            u32 rangeIndex)
    {
        slang::BindingType bindingType = typeLayout->getDescriptorSetDescriptorRangeType(relativeSetIndex, rangeIndex);
        auto descriptorCount = typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(relativeSetIndex, rangeIndex);

        if (bindingType == slang::BindingType::PushConstant)
            return;

        auto bindingIndex = descriptorSetLayoutBuilder.descriptorRanges.size();

        VkDescriptorSetLayoutBinding vkBindingRange = {};
        vkBindingRange.binding = bindingIndex;
        vkBindingRange.descriptorCount = descriptorCount;
        vkBindingRange.stageFlags = VK_SHADER_STAGE_ALL; //TODO: JUST USE RELEVANT STAGES
        vkBindingRange.descriptorType = MapSlangBindingTypeToVulkanDescriptorType(bindingType);


        descriptorSetLayoutBuilder.descriptorRanges.push_back(vkBindingRange);
    }

    void AddDescriptorRanges(DescriptorSetLayoutBuilder& descriptorSetLayoutBuilder,
                             slang::TypeLayoutReflection* typeLayout)
    {
        u32 relativeSetIndex = 0;
        u32 rangeCount = typeLayout->getDescriptorSetDescriptorRangeCount(relativeSetIndex);

        for (u32 rangeIndex = 0; rangeIndex < rangeCount; rangeIndex++)
        {
            AddDescriptorRange(descriptorSetLayoutBuilder, typeLayout, relativeSetIndex, rangeIndex);
        }
    }

    void AddAutomaticallyIntroducedUniformBuffer(DescriptorSetLayoutBuilder& descriptorSetLayoutBuilder)
    {
        auto vulkanBindingIndex = descriptorSetLayoutBuilder.descriptorRanges.size();

        VkDescriptorSetLayoutBinding binding = {};
        binding.stageFlags = VK_SHADER_STAGE_ALL;
        binding.binding = vulkanBindingIndex;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        descriptorSetLayoutBuilder.descriptorRanges.push_back(binding);
    }

    void AddDescriptorSetForParameterBlock(
        PipelineLayoutBuilder& pipelineLayoutBuilder,
        slang::TypeLayoutReflection* parameterBlockTypeLayout);

    void AddPushConstantRangeForConstantBuffer(PipelineLayoutBuilder& pipelineLayoutBuilder,
                                               slang::TypeLayoutReflection* pushConstantBufferTypeLayout)
    {
        auto elementTypeLayout = pushConstantBufferTypeLayout->getElementTypeLayout();
        auto elementSize = elementTypeLayout->getSize();

        if (elementSize == 0)
            return;

        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
        pushConstantRange.offset = 0;
        pushConstantRange.size = elementSize;

        pipelineLayoutBuilder.pushConstantRanges.push_back(pushConstantRange);
    }

    void AddSubObjectRange(PipelineLayoutBuilder& pipelineLayoutBuilder, slang::TypeLayoutReflection* type_layout,
                           u32 subObjectRangeIndex)
    {
        auto bindingRangeIndex = type_layout->getSubObjectRangeBindingRangeIndex(subObjectRangeIndex);
        auto bindingType = type_layout->getBindingRangeType(subObjectRangeIndex);
        switch (bindingType)
        {
        default:
            return;

        case slang::BindingType::ParameterBlock:
            {
                auto parameterBlockTypeLayout =
                    type_layout->getBindingRangeLeafTypeLayout(bindingRangeIndex);
                AddDescriptorSetForParameterBlock(pipelineLayoutBuilder, parameterBlockTypeLayout);
            }
            break;

        case slang::BindingType::PushConstant:
            {
                auto constantBufferTypeLayout = type_layout->getBindingRangeLeafTypeLayout(bindingRangeIndex);
                AddPushConstantRangeForConstantBuffer(pipelineLayoutBuilder, constantBufferTypeLayout);
            }
            break;
        }
    }

    void AddSubObjectRanges(PipelineLayoutBuilder& pipelineLayoutBuilder,
                            slang::TypeLayoutReflection* typeLayout)
    {
        u32 subObjectRangeCount = typeLayout->getSubObjectRangeCount();
        for (u32 subObjectIndex = 0; subObjectIndex < subObjectRangeCount; subObjectIndex++)
        {
            AddSubObjectRange(pipelineLayoutBuilder, typeLayout, subObjectIndex);
        }
    }

    //Sub-object ranges
    void AddRanges(PipelineLayoutBuilder& pipelineLayoutBuilder,
                   DescriptorSetLayoutBuilder& descriptorSetLayoutBuilder,
                   slang::TypeLayoutReflection* typeLayout)
    {
        AddDescriptorRanges(descriptorSetLayoutBuilder, typeLayout);
        AddSubObjectRanges(pipelineLayoutBuilder, typeLayout);
    }

    void AddRangesForParameterBlockElement(PipelineLayoutBuilder& pipelineLayoutBuilder,
                                           DescriptorSetLayoutBuilder& descriptorSetLayoutBuilder,
                                           slang::TypeLayoutReflection* elementTypeLayout)
    {
        if (elementTypeLayout->getSize() > 0)
        {
            AddAutomaticallyIntroducedUniformBuffer(descriptorSetLayoutBuilder);
        }

        // Once we have accounted for the possibility of an implicitly-introduced
        // constant buffer, we can move on and add bindings based on whatever
        // non-ordinary data (textures, buffers, etc.) is in the element type:
        //

        AddRanges(pipelineLayoutBuilder, descriptorSetLayoutBuilder, elementTypeLayout);
    }

    void FinishBuildingDescriptorSetLayout(PipelineLayoutBuilder& pipelineLayoutBuilder,
                                           DescriptorSetLayoutBuilder& descriptorSetLayoutBuilder)
    {

    }

    void AddDescriptorSetForParameterBlock(PipelineLayoutBuilder& pipelineLayoutBuilder,
                                           slang::TypeLayoutReflection* parameterBlockTypeLayout)
    {
        DescriptorSetLayoutBuilder descriptorSetLayoutBuilder;

        StartBuildingDescriptorSetLayout(pipelineLayoutBuilder, descriptorSetLayoutBuilder);
        AddRangesForParameterBlockElement(pipelineLayoutBuilder, descriptorSetLayoutBuilder,
                                          parameterBlockTypeLayout->getElementTypeLayout());


        FinishBuildingDescriptorSetLayout(pipelineLayoutBuilder, descriptorSetLayoutBuilder);
    }

    void AddGlobalScopeParameters(PipelineLayoutBuilder& pipelineLayoutBuilder,
        DescriptorSetLayoutBuilder& descriptorSetLayoutBuilder,
        slang::ProgramLayout* programLayout)
    {
        AddRangesForParameterBlockElement(pipelineLayoutBuilder,
            descriptorSetLayoutBuilder,
            programLayout->getGlobalParamsTypeLayout());
    }

    void AddEntryPointParameters(
       PipelineLayoutBuilder& pipelineLayoutBuilder,
       DescriptorSetLayoutBuilder& descriptorSetLayoutBuilder,
       slang::EntryPointLayout* entryPointLayout)
    {
        AddRangesForParameterBlockElement(
            pipelineLayoutBuilder,
            descriptorSetLayoutBuilder,
            entryPointLayout->getTypeLayout());
    }

    void AddEntryPointsParameters(PipelineLayoutBuilder& pipelineLayoutBuilder,
        DescriptorSetLayoutBuilder& descriptorSetLayoutBuilder,
        slang::ProgramLayout* programLayout)
    {
        u32 entryPointCount = programLayout->getEntryPointCount();
        for (u32 i =0; i<entryPointCount; i++)
        {
            auto entryPointLayout = programLayout->getEntryPointByIndex(i);
            AddEntryPointParameters(pipelineLayoutBuilder,descriptorSetLayoutBuilder,entryPointLayout);
        }
    }

    Result CreatePipelineLayout(slang::ProgramLayout* programLayout,
                                VkPipelineLayout* outPipelineLayout)
    {
        PipelineLayoutBuilder pipelineLayoutBuilder;
        DescriptorSetLayoutBuilder descriptorSetLayoutBuilder;
        StartBuildingDescriptorSetLayout(pipelineLayoutBuilder, descriptorSetLayoutBuilder);

        AddGlobalScopeParameters(pipelineLayoutBuilder, descriptorSetLayoutBuilder, programLayout);

        AddEntryPointsParameters(pipelineLayoutBuilder, descriptorSetLayoutBuilder, programLayout);

        FinishBuildingDescriptorSetLayout(pipelineLayoutBuilder, descriptorSetLayoutBuilder);

        FilterOutEmptyDescriptorSets(pipelineLayoutBuilder);

        return SLANG_OK;
    }

    void GetProgramsDescriptorSets(slang::ProgramLayout* programLayout, ShaderCompilation& outShaderCompilation)
    {
        PipelineLayoutBuilder pipelineLayoutBuilder;
        DescriptorSetLayoutBuilder descriptorSetLayoutBuilder;

        StartBuildingDescriptorSetLayout(pipelineLayoutBuilder, descriptorSetLayoutBuilder);
        AddGlobalScopeParameters(pipelineLayoutBuilder, descriptorSetLayoutBuilder, programLayout);
        AddEntryPointsParameters(pipelineLayoutBuilder, descriptorSetLayoutBuilder, programLayout);
        FinishBuildingDescriptorSetLayout(pipelineLayoutBuilder, descriptorSetLayoutBuilder);
        FilterOutEmptyDescriptorSets(pipelineLayoutBuilder);

        for (auto descriptor_range : descriptorSetLayoutBuilder.descriptorRanges)
        {

        }

    }

}

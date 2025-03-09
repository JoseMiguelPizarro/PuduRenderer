//
// Created by Hojaverde on 2/23/2025.
//


#include <algorithm>
#include <ranges>
#include <slang/slang.h>
#include "ShaderCompilation/ShaderObjectLayoutBuilder.h"

#undef  VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vk_enum_string_helper.h"
#define VK_USE_PLATFORM_WIN32_KHR

#include <boolinq.h>
#include <map>
#include <fastgltf/types.hpp>


#include "Logger.h"

namespace Pudu
{
    static std::map<slang::TypeReflection::Kind, const char*> KIND_NAMES = {
        {slang::TypeReflection::Kind::None, "None"},
        {slang::TypeReflection::Kind::Struct, "Struct"},
        {slang::TypeReflection::Kind::Array, "Array"},
        {slang::TypeReflection::Kind::Matrix, "Matrix"},
        {slang::TypeReflection::Kind::Vector, "Vector"},
        {slang::TypeReflection::Kind::Scalar, "Scalar"},
        {slang::TypeReflection::Kind::ConstantBuffer, "ConstantBuffer"},
        {slang::TypeReflection::Kind::TextureBuffer, "TextureBuffer"},
        {slang::TypeReflection::Kind::ShaderStorageBuffer, "ShaderStorageBuffer"},
        {slang::TypeReflection::Kind::SamplerState, "SamplerState"},
        {slang::TypeReflection::Kind::Resource, "Resource"},
        {slang::TypeReflection::Kind::ParameterBlock, "ParameterBlock"},
        {slang::TypeReflection::Kind::GenericTypeParameter, "GenericTypeParameter"},
        {slang::TypeReflection::Kind::Interface, "Interface"}
    };

    static std::map<slang::BindingType, const char*> BINDING_NAMES = {
        {
            BindingType::Unknown, "Unknown"
        },
        {BindingType::Sampler, "Sampler"},
        {BindingType::Texture, "Texture"},
        {BindingType::ConstantBuffer, "ConstantBuffer"},
        {BindingType::ParameterBlock, "ParameterBlock"},
        {BindingType::TypedBuffer, "TypedBuffer"},
        {BindingType::RawBuffer, "RawBuffer"},
        {BindingType::CombinedTextureSampler, "CombinedTextureSampler"},
        {BindingType::InputRenderTarget, "InputRenderTarget"},
        {BindingType::InlineUniformData, "InlineUniformData"},
        {BindingType::RayTracingAccelerationStructure, "RayTracingAccelerationStructure"},
        {BindingType::VaryingInput, "VaryingInput"},
        {BindingType::VaryingOutput, "VaryingOutput"},
        {BindingType::ExistentialValue, "ExistentialValue"},
        {BindingType::PushConstant, "PushConstant"},
        {BindingType::MutableFlag, "MutableFlag"},
        {BindingType::MutableTexture, "MutableTexture"},
        {BindingType::MutableTypedBuffer, "MutableTypedBuffer"},
        {BindingType::MutableRawBuffer, "MutableRawBuffer"},
        {BindingType::BaseMask, "BaseMask"},
        {BindingType::ExtMask, "ExtMask"}
    };

    static std::map<slang::ParameterCategory, const char*> PARAMETER_CATEGORY_NAMES = {
        {slang::ParameterCategory::None, "None"},
        {slang::ParameterCategory::Uniform, "Uniform"},
        {slang::ParameterCategory::ShaderResource, "ShaderResource"},
        {slang::ParameterCategory::UnorderedAccess, "UnorderedAccess"},
        {slang::ParameterCategory::VaryingInput, "VaryingInput"},
        {slang::ParameterCategory::VaryingOutput, "VaryingOutput"},
        {slang::ParameterCategory::SamplerState, "SamplerState"},
        {slang::ParameterCategory::Mixed, "Mixed"},
        {slang::ParameterCategory::SpecializationConstant, "SpecializationConstant"},
        {slang::ParameterCategory::PushConstantBuffer, "PushConstantBuffer"},
        {slang::ParameterCategory::DescriptorTableSlot, "DescriptorTableSlot"},
        {slang::ParameterCategory::RegisterSpace, "RegisterSpace"},
        {slang::ParameterCategory::ConstantBuffer, "ConstantBuffer"},
        {slang::ParameterCategory::GenericResource, "GenericResource"},
        {slang::ParameterCategory::ExistentialObjectParam, "ExistentialObjectParam"},
        {slang::ParameterCategory::SubElementRegisterSpace, "SubElementRegisterSpace"}
    };

    VkDescriptorType ToVk(slang::BindingType slangBindingType)
    {
        switch (slangBindingType)
        {
        default:
            std::fprintf(stderr, "Assertion failed: %s %s\n", "Unsupported binding type!",
                         BINDING_NAMES.at(slangBindingType));
        //std::abort();
            return VK_DESCRIPTOR_TYPE_MAX_ENUM;

        case slang::BindingType::Sampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case slang::BindingType::CombinedTextureSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case slang::BindingType::Texture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case slang::BindingType::MutableTexture:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case slang::BindingType::TypedBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case slang::BindingType::MutableTypedBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case slang::BindingType::RawBuffer:
        case slang::BindingType::MutableRawBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case slang::BindingType::InputRenderTarget:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        case slang::BindingType::InlineUniformData:
            return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
        case slang::BindingType::RayTracingAccelerationStructure:
            return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        case slang::BindingType::ConstantBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
    }


    void AccessPath::Print() const
    {
        if (leaf)
        {
            AccessPathNode* currentNode = leaf;
            std::string path;
            while (currentNode)
            {
                const char* name = currentNode->variableLayout->getName()
                                       ? currentNode->variableLayout->getName()
                                       : "Root";
                path = name + (path.empty() ? "" : "->" + path);
                currentNode = currentNode->parent;
            }
            std::cout << path.c_str() << std::endl;
        }
        else
        {
            LOG("No leaf node exists in the AccessPath.");
        }
    }

    ConstantBufferInfo* ShaderLayoutBuilderContext::PushConstantBufferInfo()
    {
        m_constantBuffers.resize(m_constantBuffers.size() + 1);
        return &m_constantBuffers.back();
    }

    ConstantBufferInfo* ShaderLayoutBuilderContext::PushPushConstantsBufferInfo()
    {
        m_pushConstants.resize(m_pushConstants.size() + 1);
        return &m_pushConstants.back();
    }

    std::vector<ConstantBufferInfo>& ShaderLayoutBuilderContext::GetPushConstants()
    {
        return m_pushConstants;
    }

    std::vector<ConstantBufferInfo>* ShaderLayoutBuilderContext::GetConstantBufferInfos()
    {
        return &m_constantBuffers;
    }

    void ShaderLayoutBuilderContext::PushBinding(const DescriptorBinding& binding) const
    {
        shaderCompilationObject->descriptorsData.bindingsData.push_back(binding);
    }


    void ShaderObjectLayoutBuilder::ParseVariableTypeLayout(TypeLayoutReflection* typeLayoutReflection,
                                                            ShaderLayoutBuilderContext* context, AccessPath accessPath)
    {
        m_indentation++;
        auto kind = typeLayoutReflection->getKind();
        LOG_I(m_indentation, "KIND: {}", KIND_NAMES.at(kind));
        switch (kind)
        {
        case TypeReflection::Kind::Struct:
            {
                size fieldCount = typeLayoutReflection->getFieldCount();

                for (size_t i = 0; i < fieldCount; i++)
                {
                    auto field = typeLayoutReflection->getFieldByIndex(i);
                    ParseVariableLayout(field, context, accessPath);
                }
            }
            break;

        case TypeReflection::Kind::ConstantBuffer:
        case TypeReflection::Kind::ParameterBlock:
        case TypeReflection::Kind::TextureBuffer:
        case TypeReflection::Kind::ShaderStorageBuffer:
            {
                auto containerVarLayout = typeLayoutReflection->getContainerVarLayout();
                auto elementVarLayout = typeLayoutReflection->getElementVarLayout();

                AccessPath innerOffsets = accessPath;
                accessPath.rootBufferInfo = context->PushConstantBufferInfo();
                accessPath.rootBufferInfo->shaderStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT |
                    VK_SHADER_STAGE_COMPUTE_BIT;

                //Binding stack
                Binding offsets;
                if (kind == TypeReflection::Kind::ParameterBlock)
                {
                    VariableLayoutReflection* container = typeLayoutReflection->getContainerVarLayout();

                    context->PushSetIndex();

                    //Create New DescriptorSet
                    {
                        DescriptorSetLayoutInfo descriptorSetLayoutData;
                        descriptorSetLayoutData.SetNumber = context->getSetIndex();
                        descriptorSetLayoutData.name = accessPath.leaf->variableLayout->getName();
                        descriptorSetLayoutData.CreateInfo.pBindings = descriptorSetLayoutData.Bindings.data();
                        descriptorSetLayoutData.CreateInfo.flags = 0;

                        if (auto bindlessAttribute = container->getVariable()->findUserAttributeByName(
                            m_globalSession, "Bindless"))
                        {
                            int v;
                            descriptorSetLayoutData.bindless = bindlessAttribute->getArgumentValueInt(0, &v);
                        }

                        context->shaderCompilationObject->descriptorsData.setsCount++;
                        context->shaderCompilationObject->descriptorsData.layoutData.push_back(descriptorSetLayoutData);
                    }

                    accessPath.setIndex = context->getSetIndex();
                    accessPath.cumulativeOffset = &offsets;

                    auto firstCategory = container->getCategoryByIndex(0);
                    auto categoryCount = container->getCategoryCount();

                    for (size_t i = 0; i < categoryCount; i++)
                    {
                        auto category = container->getCategoryByIndex(i);
                        LOG_I(m_indentation, "ParameterBlockCategory: {}", PARAMETER_CATEGORY_NAMES.at(category));
                    }

                    LOG_I(m_indentation, "First category: {}", PARAMETER_CATEGORY_NAMES.at(firstCategory));
                    if (firstCategory == slang::ParameterCategory::DescriptorTableSlot)
                    {
                        //Implicitly allocated uniform buffer
                        accessPath.cumulativeOffset->PushIndex();
                        DescriptorBinding binding;
                        binding.count = 1;
                        binding.index = accessPath.cumulativeOffset->index;
                        binding.setNumber = accessPath.setIndex;
                        binding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        binding.name = accessPath.leaf->variableLayout->getName();

                        accessPath.rootBufferInfo->bindingIndex = accessPath.cumulativeOffset->index;
                        accessPath.rootBufferInfo->setNumber = accessPath.setIndex;

                        context->PushBinding(binding);
                    }
                }
                else if(!accessPath.isPushConstant)
                {
                    //PUSH DESCRIPTOR SET FOR BUFFER
                    accessPath.cumulativeOffset->PushIndex();
                    LOG_I(m_indentation, "Set: {} Binding: {}", accessPath.setIndex,
                          accessPath.cumulativeOffset->index);

                    DescriptorBinding binding;
                    binding.setNumber = accessPath.setIndex;
                    binding.index = accessPath.cumulativeOffset->index;
                    binding.count = 1;
                    binding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    binding.name = accessPath.leaf->variableLayout->getName();

                    accessPath.rootBufferInfo->bindingIndex = accessPath.cumulativeOffset->index;
                    accessPath.rootBufferInfo->setNumber = accessPath.setIndex;
                    context->PushBinding(binding);
                }

                ExtendedAccessPath elementOffsets(innerOffsets, elementVarLayout);
                ParseVariableTypeLayout(elementVarLayout->getTypeLayout(), context, accessPath);
            }
            break;

        ///Here we should push a binding
        case TypeReflection::Kind::Resource:
            {
                auto element = typeLayoutReflection->getElementVarLayout();

                accessPath.cumulativeOffset->PushIndex();

                LOG_I(m_indentation, "{} Set: {} Binding: {}", element->getName() ? element->getName() : "",
                      accessPath.setIndex,
                      accessPath.cumulativeOffset->index);
                auto descriptorType = string_VkDescriptorType(ToVk(typeLayoutReflection->getBindingRangeType(0)));
                LOG_I(m_indentation, "Descriptor type: {}", descriptorType);

                DescriptorBinding binding;
                binding.type = ToVk(typeLayoutReflection->getBindingRangeType(0));
                binding.index = accessPath.cumulativeOffset->index;
                binding.setNumber = accessPath.setIndex;
                binding.count = 1;
                binding.name = typeLayoutReflection->getName();
                context->PushBinding(binding);
            }
            break;
        //Here we should push a binding
        case TypeReflection::Kind::Scalar:
        case TypeReflection::Kind::Vector:
        case TypeReflection::Kind::Matrix:
            {
                DescriptorBinding binding;

                auto container = typeLayoutReflection->getContainerVarLayout();
                LOG_I(m_indentation, "Container: {}", KIND_NAMES.at(container->getType()->getKind()));

                LOG_I(m_indentation, "Set: {} Binding: {}", accessPath.setIndex, accessPath.cumulativeOffset->index);
            }
            break;
        case TypeReflection::Kind::Array:
            {
                //TODO: How to handle array of resources?
                auto arrayKind = accessPath.leaf->variableLayout->getType()->getElementType()->getKind();
                LOG_I(m_indentation, "Array Kind {} ElementCount: {} Size {} :", KIND_NAMES.at(arrayKind),
                      typeLayoutReflection->getElementCount(),
                      typeLayoutReflection->getSize());

                if (arrayKind == TypeReflection::Kind::Resource)
                {
                    ParseVariableTypeLayout(typeLayoutReflection->unwrapArray(), context, accessPath);
                }
            }
            break;

        default: break;
        }
        m_indentation--;
    }

    void ShaderObjectLayoutBuilder::ParseVariableLayout(VariableLayoutReflection* varLayout,
                                                        ShaderLayoutBuilderContext* context, AccessPath accessPath)
    {
        m_indentation++;

        ExtendedAccessPath varPath(accessPath, varLayout);

        ParseVariableOffsets(varLayout, context, static_cast<AccessPath>(varPath));

        m_indentation--;
    }

    void ShaderObjectLayoutBuilder::ParseVariableOffsets(VariableLayoutReflection* varLayout,
                                                         ShaderLayoutBuilderContext* context, AccessPath accessPath)
    {
        m_indentation++;
        size usedLayoutUnitsCount = varLayout->getCategoryCount();
        accessPath.Print();

        for (size_t i = 0; i < usedLayoutUnitsCount; i++)
        {
            auto category = varLayout->getCategoryByIndex(i);
            LOG_I(m_indentation, "Unit type: {}", PARAMETER_CATEGORY_NAMES.at(category));

            switch (category)
            {
            case PushConstantBuffer:
                {
                    accessPath.rootBufferInfo = context->PushPushConstantsBufferInfo();
                    accessPath.rootBufferInfo->shaderStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
                        | VK_SHADER_STAGE_COMPUTE_BIT;
                    accessPath.setIndex = 0;
                    accessPath.cumulativeOffset->PushIndex();
                    accessPath.rootBufferInfo->bindingIndex = 0;
                    accessPath.rootBufferInfo->setNumber = 0;
                    accessPath.rootBufferInfo->name = varLayout->getName();

                    accessPath.isPushConstant = true;
                }
                break;
            case Uniform:
                {
                    LOG_I(m_indentation, "Size:{} Set: {} Binding: {}", varLayout->getTypeLayout()->getSize(),
                          accessPath.setIndex,
                          0);
                    accessPath.rootBufferInfo->PushElement(varLayout->getTypeLayout()->getSize());
                    LOG_I(m_indentation, "ConstantBufferSize {}", accessPath.rootBufferInfo->size);
                }
                break;
            case ConstantBuffer:
            case slang::ParameterCategory::ShaderResource:
            case slang::ParameterCategory::UnorderedAccess:
            case slang::ParameterCategory::SamplerState:
            case slang::ParameterCategory::DescriptorTableSlot:
                {
                    //    LOG_I(m_indentation, "set: {} Binding: {}", accessPath.m_setIndex, accessPath.cumulativeOffset->index);
                }
                break;
            default: break;
            }
        }

        ParseVariableTypeLayout(varLayout->getTypeLayout(), context, static_cast<AccessPath>(accessPath));

        m_indentation--;
    }

    void ShaderObjectLayoutBuilder::ParseScope(slang::VariableLayoutReflection* scopeVarLayout,
                                               ShaderLayoutBuilderContext* context, AccessPath accessPath)
    {
        m_indentation++;
        ExtendedAccessPath scopeOffsets(accessPath, scopeVarLayout);

        TypeLayoutReflection* scopeTypeLayout = scopeVarLayout->getTypeLayout();

        scopeOffsets.rootBufferInfo = context->PushConstantBufferInfo();
        scopeOffsets.rootBufferInfo->shaderStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT |
            VK_SHADER_STAGE_COMPUTE_BIT;


        Binding cumulativeOffset;
        scopeOffsets.cumulativeOffset = &cumulativeOffset;

        switch (auto scopeKind = scopeTypeLayout->getKind())
        {
        case TypeReflection::Kind::Struct:
            {
                size paramCount = scopeTypeLayout->getFieldCount();
                for (u16 i = 0; i < paramCount; i++)
                {
                    auto param = scopeTypeLayout->getFieldByIndex(i);
                    ParseVariableLayout(param, context, static_cast<AccessPath>(scopeOffsets));
                }
            }
            break;
        case TypeReflection::Kind::ConstantBuffer:
            {
                //Constant buffer detected to add
                LOG_I(m_indentation, "Parsing Constant Buffer");
                auto containerLayout = scopeTypeLayout->getContainerVarLayout();
                ParseScope(scopeTypeLayout->getElementVarLayout(), context, static_cast<AccessPath>(scopeOffsets));
                ConstantBufferInfo constantBufferInfo;
            }
            break;
        case TypeReflection::Kind::ParameterBlock:
            LOG_I(m_indentation, "Parsing ParameterBlock");
            ParseScope(scopeTypeLayout->getElementVarLayout(), context, static_cast<AccessPath>(scopeOffsets));
            break;
        default:
            break;
        }
        m_indentation--;
    }

    void ShaderObjectLayoutBuilder::ParseShaderProgramLayout(slang::ProgramLayout* programLayout,
                                                             ShaderCompilationObject& outCompilationObject)
    {
        m_indentation = 0;

        auto globalVarLayout = programLayout->getGlobalParamsVarLayout();

        AccessPath rootOffsets;
        rootOffsets.valid = true;

        ShaderLayoutBuilderContext context;
        context.shaderCompilationObject = &outCompilationObject;

        ParseScope(globalVarLayout, &context, rootOffsets);

        std::vector<ConstantBufferInfo> buffersToAllocate;
        std::vector<DescriptorSetLayoutInfo> descriptorSetsToAllocate;

        auto& constantBuffers = *context.GetConstantBufferInfos();

        for (auto& cbuffer : constantBuffers)
        {
            if (cbuffer.size > 0)
            {
                buffersToAllocate.push_back(cbuffer);
            }
        }

        outCompilationObject.descriptorsData.setsCount = context.getSetIndex() + 1;


        outCompilationObject.SetPushConstants(context.GetPushConstants());
        outCompilationObject.SetBuffersToAllocate(buffersToAllocate);

        for (auto& binding : outCompilationObject.descriptorsData.bindingsData)
        {
            ASSERT(binding.setNumber < outCompilationObject.descriptorsData.layoutData.size(),
                   "Binding set out of bounds name: {} set: {}",
                   binding.name, binding.setNumber);
            auto& layout = outCompilationObject.descriptorsData.layoutData[binding.setNumber];

            layout.Bindings.push_back(binding.ToVKDescriptorSetLayoutBinding());
        }

        //Setup layout create info
        for (auto& layout : outCompilationObject.descriptorsData.layoutData)
        {
            ASSERT(layout.Bindings.size() <=16, "Maximum binding count exceded for layout: {} bindings count: {}",layout.name, layout.Bindings.size() );
            layout.CreateInfo.bindingCount = layout.Bindings.size();
            layout.CreateInfo.pBindings = layout.Bindings.data();
            layout.CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            layout.CreateInfo.pNext = nullptr;
            layout.CreateInfo.flags = 0;
        }
    }
} // Pudu

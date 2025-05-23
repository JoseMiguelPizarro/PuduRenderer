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
            ASSERT(false, "Invalid Binding Type");
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

    ShaderLayoutBuilderContext::ShaderLayoutBuilderContext()
    {
        m_constantBuffers.reserve(MAX_BUFFERS_COUNT);
        m_pushConstants.reserve(MAX_BUFFERS_COUNT);
    }

    ConstantBufferInfo* ShaderLayoutBuilderContext::PushConstantBufferInfo()
    {
        ASSERT(m_constantBuffers.size() < MAX_BUFFERS_COUNT, "MAX CONSTANT BUFFERS EXCEEDED {}", MAX_BUFFERS_COUNT);
        m_constantBuffers.resize(m_constantBuffers.size() + 1);
        return &m_constantBuffers.back();
    }

    ConstantBufferInfo* ShaderLayoutBuilderContext::PushPushConstantsBufferInfo()
    {
        ASSERT(m_pushConstants.size() < MAX_BUFFERS_COUNT, "MAX PUSH CONSTANT BUFFERS EXCEEDED {}", MAX_BUFFERS_COUNT);

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
        switch (kind)
        {
        case TypeReflection::Kind::Struct:
            {
                size fieldCount = typeLayoutReflection->getFieldCount();

                if (!accessPath.isContainerStructDefinition)
                {
                    auto shaderNode = accessPath.shaderNode->AppendChild(
                        accessPath.leaf->variableLayout->getName(), 0, 0,
                        ShaderNodeType::Struct);
                    shaderNode->setIndex = accessPath.setIndex;
                    shaderNode->bindingIndex = 0;
                    accessPath.shaderNode = shaderNode;
                }

                accessPath.isContainerStructDefinition = false;

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
                auto elementVarLayout = typeLayoutReflection->getElementVarLayout();

                accessPath.rootBufferInfo = context->PushConstantBufferInfo();
                accessPath.rootBufferInfo->shaderStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT |
                    VK_SHADER_STAGE_COMPUTE_BIT;

                accessPath.isContainerStructDefinition = true;
                /////////////
                //Binding stack
                Binding offsets;
                if (kind == TypeReflection::Kind::ParameterBlock)
                {
                    VariableLayoutReflection* container = typeLayoutReflection->getContainerVarLayout();

                    accessPath.isContainerStructDefinition = true;
                    context->PushSetIndex();

                    //Create New DescriptorSet
                    {
                        DescriptorSetLayoutInfo descriptorSetLayoutInfo;
                        descriptorSetLayoutInfo.SetNumber = context->getSetIndex();
                        descriptorSetLayoutInfo.name = accessPath.leaf->variableLayout->getName();
                        descriptorSetLayoutInfo.CreateInfo.pBindings = descriptorSetLayoutInfo.Bindings.data();
                        descriptorSetLayoutInfo.CreateInfo.flags = 0;

                        auto reflectedVar = accessPath.leaf->variableLayout->getVariable();
                        if (auto bindlessAttribute = reflectedVar->findUserAttributeByName(m_globalSession,
                            "Bindless"))
                        {
                            descriptorSetLayoutInfo.bindless = true;
                        }

                        descriptorSetLayoutInfo.scope = "";
                        auto scope = std::string("");
                        if (auto scopeAttribute = reflectedVar->findUserAttributeByName(m_globalSession, "Scope"))
                        {
                            Size stringSize = 0;
                            scope = scopeAttribute->getArgumentValueString(0, &stringSize);
                            if (stringSize > 0)
                                scope = scope.substr(1, stringSize - 2);
                            //Hack since Slang API doen'st return the real string but the whole code

                            descriptorSetLayoutInfo.scope = scope;
                        }

                        context->shaderCompilationObject->descriptorsData.setsCount++;
                        context->shaderCompilationObject->descriptorsData.setLayoutInfos.push_back(
                            descriptorSetLayoutInfo);

                        auto shaderNode = accessPath.shaderNode->AppendChild(
                            descriptorSetLayoutInfo.name.c_str(), 0, 0, ShaderNodeType::ParameterBlock);

                        shaderNode->scope = scope;
                        shaderNode->setIndex = descriptorSetLayoutInfo.SetNumber;

                        accessPath.shaderNode = shaderNode;
                    }

                    accessPath.setIndex = context->getSetIndex();
                    accessPath.cumulativeOffset = &offsets;

                    auto firstCategory = container->getCategoryByIndex(0);

                    if (firstCategory == slang::ParameterCategory::DescriptorTableSlot)
                    {
                        //Implicitly allocated uniform buffer
                        accessPath.cumulativeOffset->PushIndex();
                        DescriptorBinding binding;
                        binding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        binding.name = "_CBuffer";
                        binding.count = 1;
                        binding.index = accessPath.cumulativeOffset->index;
                        binding.setNumber = accessPath.setIndex;
                        context->PushBinding(binding);

                        accessPath.rootBufferInfo->bindingIndex = accessPath.cumulativeOffset->index;
                        accessPath.rootBufferInfo->setNumber = accessPath.setIndex;

                        if (accessPath.rootBufferShaderNode == nullptr)
                        {
                            auto shaderNode = accessPath.shaderNode->AppendChild(
                                binding.name.c_str(), 0, 0, ShaderNodeType::CBuffer);

                            shaderNode->binding = binding;
                            shaderNode->setIndex = accessPath.setIndex;
                            shaderNode->bindingIndex = accessPath.cumulativeOffset->index;
                            shaderNode->name = "_CBuffer";
                            shaderNode->scope = accessPath.shaderNode->scope;

                            accessPath.rootBufferShaderNode = shaderNode;
                        }
                    }
                }
                else if (!accessPath.isPushConstant)
                {
                    //🐞 PUSH DESCRIPTOR SET FOR A REGULAR BUFFER
                    accessPath.cumulativeOffset->PushIndex();

                    DescriptorBinding binding;
                    binding.setNumber = accessPath.setIndex;
                    binding.index = accessPath.cumulativeOffset->index;
                    binding.count = 1;
                    binding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    binding.name = accessPath.leaf->variableLayout->getName();

                    accessPath.rootBufferInfo->bindingIndex = accessPath.cumulativeOffset->index;
                    accessPath.rootBufferInfo->setNumber = accessPath.setIndex;
                    context->PushBinding(binding);

                    auto shaderNode = accessPath.shaderNode->AppendChild(
                        binding.name.c_str(), 0, 0, ShaderNodeType::Buffer);

                    shaderNode->setIndex = accessPath.setIndex;
                    shaderNode->bindingIndex = accessPath.cumulativeOffset->index;
                    shaderNode->binding = binding;
                    accessPath.shaderNode = shaderNode;
                    accessPath.rootBufferShaderNode = shaderNode;
                }

                ParseVariableTypeLayout(elementVarLayout->getTypeLayout(), context, accessPath);
            }
            break;

        ///Here we should push a binding
        case TypeReflection::Kind::Resource:
            {
                accessPath.cumulativeOffset->PushIndex();

                DescriptorBinding binding;
                binding.type = ToVk(typeLayoutReflection->getBindingRangeType(0));
                binding.index = accessPath.cumulativeOffset->index;
                binding.setNumber = accessPath.setIndex;
                binding.count = 1;
                binding.name = accessPath.leaf->variableLayout->getName();
                context->PushBinding(binding);

                auto shaderNode = accessPath.shaderNode->AppendChild(binding.name.c_str(), 0, 0,
                                                                     ShaderNodeType::Resource);
                shaderNode->setIndex = accessPath.setIndex;
                shaderNode->bindingIndex = accessPath.cumulativeOffset->index;
                shaderNode->binding = binding;
                accessPath.shaderNode = shaderNode;
            }
            break;
        //Here we should push a binding
        case TypeReflection::Kind::Scalar:
        case TypeReflection::Kind::Vector:
        case TypeReflection::Kind::Matrix:
            {
                auto node = accessPath.shaderNode->AppendChild(
                    accessPath.leaf->variableLayout->getName(), accessPath.leaf->variableLayout->getOffset(),
                    typeLayoutReflection->getStride(), ShaderNodeType::Uniform);

                node->setIndex = accessPath.setIndex;
                node->bindingIndex = accessPath.cumulativeOffset->index;
                node->parentContainer = accessPath.rootBufferShaderNode;
            }
            break;
        case TypeReflection::Kind::Array:
            {
                //TODO: How to handle array of resources?
                auto arrayKind = accessPath.leaf->variableLayout->getType()->getElementType()->getKind();

                if (arrayKind == TypeReflection::Kind::Resource)
                {
                    accessPath.cumulativeOffset->PushIndex();

                    DescriptorBinding binding;
                    binding.type = ToVk(typeLayoutReflection->getBindingRangeType(0));
                    binding.index = accessPath.cumulativeOffset->index;
                    binding.setNumber = accessPath.setIndex;
                    binding.count = typeLayoutReflection->getElementCount();
                    binding.name = accessPath.leaf->variableLayout->getName();
                    context->PushBinding(binding);

                    auto shaderNode = accessPath.shaderNode->AppendChild(
                        accessPath.leaf->variableLayout->getName(),
                        accessPath.leaf->variableLayout->getOffset(),
                        typeLayoutReflection->getSize(), ShaderNodeType::Array);

                    shaderNode->setIndex = accessPath.setIndex;
                    shaderNode->bindingIndex = accessPath.cumulativeOffset->index;
                    shaderNode->elementCount = typeLayoutReflection->getElementCount();
                    shaderNode->binding = binding;

                    accessPath.shaderNode = shaderNode;
                    // ParseVariableTypeLayout(typeLayoutReflection->unwrapArray(), context, accessPath);
                }
                else //Scalar, we should add it to the CBuffer
                {
                    auto node = accessPath.rootBufferShaderNode->AppendChild(
                        accessPath.leaf->variableLayout->getName(), accessPath.leaf->variableLayout->getOffset(),
                        typeLayoutReflection->unwrapArray()->getStride(),
                        ShaderNodeType::Array);

                    node->setIndex = accessPath.setIndex;
                    node->bindingIndex = accessPath.cumulativeOffset->index;
                    node->elementCount = typeLayoutReflection->getElementCount();
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

        for (size_t i = 0; i < usedLayoutUnitsCount; i++)
        {
            auto category = varLayout->getCategoryByIndex(i);

            switch (category)
            {
            case slang::ParameterCategory::PushConstantBuffer:
                {
                    accessPath.rootBufferInfo = context->PushPushConstantsBufferInfo();
                    accessPath.rootBufferInfo->shaderStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; //TODO: USE REAL RANGES
                    accessPath.setIndex = 0;
                    accessPath.cumulativeOffset->PushIndex();
                    accessPath.rootBufferInfo->bindingIndex = 0;
                    accessPath.rootBufferInfo->setNumber = 0;
                    accessPath.rootBufferInfo->name = varLayout->getName();

                    accessPath.isPushConstant = true;

                    auto shaderNode = accessPath.shaderNode->AppendChild(
                        varLayout->getName(), 0, 0, ShaderNodeType::PushConstant);

                    accessPath.shaderNode = shaderNode;
                    accessPath.rootBufferShaderNode = shaderNode;
                    accessPath.isContainerStructDefinition = true;
                }
                break;
            case slang::ParameterCategory::Uniform:
                {
                    ASSERT(accessPath.rootBufferShaderNode != nullptr, "Root buffer shader node null for {}",
                           varLayout->getName());

                    accessPath.rootBufferInfo->PushElement(varLayout->getTypeLayout()->getSize());
                    accessPath.rootBufferShaderNode->size = accessPath.rootBufferInfo->size;
                }
                break;
            default: break;
            }
        }

        const auto l = varLayout->getTypeLayout();

        ASSERT(l != nullptr, "TypeLayout is null for {}", varLayout->getName());

        ParseVariableTypeLayout(varLayout->getTypeLayout(), context, accessPath);


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
                ParseScope(scopeTypeLayout->getElementVarLayout(), context, static_cast<AccessPath>(scopeOffsets));
            }
            break;
        case TypeReflection::Kind::ParameterBlock:
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

        SPtr<ShaderNode> rootNode = std::make_shared<ShaderNode>(ShaderNode(
            ROOT_NAME, 0, 0, ShaderNodeType::Root));

        rootOffsets.shaderNode = rootNode.get();
        outCompilationObject.descriptorsData.m_shaderLayout = rootNode;

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
            ASSERT(binding.setNumber < outCompilationObject.descriptorsData.setLayoutInfos.size(),
                   "Binding set out of bounds name: {} set: {}",
                   binding.name, binding.setNumber);
            auto& layout = outCompilationObject.descriptorsData.setLayoutInfos[binding.setNumber];

            layout.Bindings.push_back(binding.ToVKDescriptorSetLayoutBinding());
        }

        //🐞 SetupPushConstants
        PushConstantInfo pushConstants {};
        for (const auto& pushBuffer : context.GetPushConstants() )
        {
            VkPushConstantRange pushConstantRange;
            pushConstantRange.offset = pushBuffer.offset;
            pushConstantRange.size = pushBuffer.size;
            pushConstantRange.stageFlags = pushBuffer.shaderStages;

            pushConstants.ranges.push_back(pushConstantRange);
        }

        outCompilationObject.m_pushConstantsInfo = pushConstants;

        //🐞 Setup layout create info
        for (auto& layout : outCompilationObject.descriptorsData.setLayoutInfos)
        {
            ASSERT(layout.Bindings.size() <= 16, "Maximum binding count exceded for layout: {} bindings count: {}",
                   layout.name, layout.Bindings.size());
            layout.CreateInfo.bindingCount = layout.Bindings.size();
            layout.CreateInfo.pBindings = layout.Bindings.data();
            layout.CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            layout.CreateInfo.pNext = nullptr;
            layout.CreateInfo.flags = 0;
        }
    }
} // Pudu

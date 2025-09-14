//
// Created by Hojaverde on 2/23/2025.
//


#include <slang/slang.h>
#include "ShaderCompilation/ShaderObjectLayoutBuilder.h"

#undef  VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vk_enum_string_helper.h"
#define VK_USE_PLATFORM_WIN32_KHR

#include <map>
#include <fastgltf/types.hpp>
#include "Logger.h"

#include "VertexLayout.h"

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
            LOG(path);
        }
        else
        {
            LOG("No leaf node exists in the AccessPath.");
        }
    }

    ShaderLayoutBuilderContext::ShaderLayoutBuilderContext()
    {
    }

    ConstantBufferInfo* ShaderLayoutBuilderContext::PushConstantBufferInfo()
    {
        ASSERT(m_constantsBufferCount < K_MAX_SHADER_BUFFER_COUNT, "MAX CONSTANT BUFFERS EXCEEDED {}",
               K_MAX_SHADER_BUFFER_COUNT);
        return &m_constantBuffers[m_constantsBufferCount++];
    }

    ConstantBufferInfo* ShaderLayoutBuilderContext::PushPushConstantsBufferInfo()
    {
        ASSERT(m_pushConstantBufferCount < K_MAX_SHADER_BUFFER_COUNT, "MAX PUSH CONSTANT BUFFERS EXCEEDED {}",
               K_MAX_SHADER_BUFFER_COUNT);

        return &m_pushConstants[m_pushConstantBufferCount++];
    }

    ConstantBufferInfo& ShaderLayoutBuilderContext::GetPushConstantsInfo(Size index)
    {
        return m_pushConstants[index];
    }

    ConstantBufferInfo& ShaderLayoutBuilderContext::GetConstantBufferInfo(Size index)
    {
        return m_constantBuffers[index];
    }

    void ShaderLayoutBuilderContext::PushBinding(const DescriptorBinding& binding) const
    {
        shaderCompilationObject->descriptorsData.bindingsData.push_back(binding);
    }

    void ShaderObjectLayoutBuilder::ParseAttributes(DescriptorSetLayoutInfo& descriptorSetLayoutInfo,
                                                    VariableReflection* reflectedVar,
                                                    ShaderLayoutBuilderContext* context)
    {
        if (auto bindlessAttribute = reflectedVar->findUserAttributeByName(m_globalSession,
                                                                           "Bindless"))
        {
            descriptorSetLayoutInfo.bindless = true;
        }

        auto scope = std::string("");
        if (auto scopeAttribute = reflectedVar->findUserAttributeByName(m_globalSession, "Scope"))
        {
            Size stringSize = 0;
            scope = scopeAttribute->getArgumentValueString(0, &stringSize);
            // if (stringSize > 0)
            // scope = scope.substr(1, stringSize - 2);

            //Hack since Slang API doen'st return the real string but the whole code

            descriptorSetLayoutInfo.scope = scope;
        }
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
                //Only allocate buffer if it's not processing a push constant
                if (accessPath.isPushConstant == false)
                {
                    accessPath.rootBufferInfo = context->PushConstantBufferInfo();
                    accessPath.rootBufferInfo->shaderStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
                        |
                        VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_GEOMETRY_BIT; //TODO: SET ONLY RELEVANT STAGES

                    accessPath.isContainerStructDefinition = true;
                }
                auto elementVarLayout = typeLayoutReflection->getElementVarLayout();

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
                        descriptorSetLayoutInfo.scope = "";
                        ParseAttributes(descriptorSetLayoutInfo, reflectedVar, context);

                        context->shaderCompilationObject->descriptorsData.setsCount++;
                        context->shaderCompilationObject->descriptorsData.setLayoutInfos.push_back(
                            descriptorSetLayoutInfo);

                        auto shaderNode = accessPath.shaderNode->AppendChild(
                            descriptorSetLayoutInfo.name.c_str(), 0, 0, ShaderNodeType::ParameterBlock);

                        shaderNode->scope = descriptorSetLayoutInfo.scope;
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
        case TypeReflection::Kind::SamplerState:
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
                shaderNode->scope = accessPath.shaderNode->scope;
                shaderNode->shape = static_cast<ShaderNode::Shape::Flags>(typeLayoutReflection->getResourceShape());

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

        default:
            ASSERT(false, "Unknown type layout kind");
            break;
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
                    //Push constant shader stage will be set at the end based on all stages present in the shader

                    //TODO: USE REAL RANGES
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
            VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_GEOMETRY_BIT; //TODO: USE RELEVANT STAGES


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


    static ChannelFormat GetChannelFormat(TypeReflection::ScalarType type, Size elementCount = 1)
    {
        switch (type)
        {
        case TypeReflection::ScalarType::Bool:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R8_UINT;
            case 2: return ChannelFormat::R8G8_UINT;
            case 3: return ChannelFormat::R8G8B8_UINT;
            case 4: return ChannelFormat::R8G8B8A8_UINT;
            default: return ChannelFormat::R8_UINT;
            }
        case TypeReflection::ScalarType::Int8:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R8_SINT;
            case 2: return ChannelFormat::R8G8_SINT;
            case 3: return ChannelFormat::R8G8B8_SINT;
            case 4: return ChannelFormat::R8G8B8A8_SINT;
            default: return ChannelFormat::R8_SINT;
            }
        case TypeReflection::ScalarType::UInt8:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R8_UINT;
            case 2: return ChannelFormat::R8G8_UINT;
            case 3: return ChannelFormat::R8G8B8_UINT;
            case 4: return ChannelFormat::R8G8B8A8_UINT;
            default: return ChannelFormat::R8_UINT;
            }
        case TypeReflection::ScalarType::Int16:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R16_SINT;
            case 2: return ChannelFormat::R16G16_SINT;
            case 3: return ChannelFormat::R16G16B16_SINT;
            case 4: return ChannelFormat::R16G16B16A16_SINT;
            default: return ChannelFormat::R16_SINT;
            }
        case TypeReflection::ScalarType::UInt16:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R16_UINT;
            case 2: return ChannelFormat::R16G16_UINT;
            case 3: return ChannelFormat::R16G16B16_UINT;
            case 4: return ChannelFormat::R16G16B16A16_UINT;
            default: return ChannelFormat::R16_UINT;
            }
        case TypeReflection::ScalarType::Int32:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R32_SINT;
            case 2: return ChannelFormat::R32G32_SINT;
            case 3: return ChannelFormat::R32G32B32_SINT;
            case 4: return ChannelFormat::R32G32B32A32_SINT;
            default: return ChannelFormat::R32_SINT;
            }
        case TypeReflection::ScalarType::UInt32:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R32_UINT;
            case 2: return ChannelFormat::R32G32_UINT;
            case 3: return ChannelFormat::R32G32B32_UINT;
            case 4: return ChannelFormat::R32G32B32A32_UINT;
            default: return ChannelFormat::R32_UINT;
            }
        case TypeReflection::ScalarType::Float32:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R32_SFLOAT;
            case 2: return ChannelFormat::R32G32_SFLOAT;
            case 3: return ChannelFormat::R32G32B32_SFLOAT;
            case 4: return ChannelFormat::R32G32B32A32_SFLOAT;
            default: return ChannelFormat::R32_SFLOAT;
            }
        case TypeReflection::ScalarType::Int64:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R64_SINT;
            case 2: return ChannelFormat::R64G64_SINT;
            case 3: return ChannelFormat::R64G64B64_SINT;
            case 4: return ChannelFormat::R64G64B64A64_SINT;
            default: return ChannelFormat::R64_SINT;
            }
        case TypeReflection::ScalarType::UInt64:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R64_UINT;
            case 2: return ChannelFormat::R64G64_UINT;
            case 3: return ChannelFormat::R64G64B64_UINT;
            case 4: return ChannelFormat::R64G64B64A64_UINT;
            default: return ChannelFormat::R64_UINT;
            }
        case TypeReflection::ScalarType::Float64:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R64_SFLOAT;
            case 2: return ChannelFormat::R64G64_SFLOAT;
            case 3: return ChannelFormat::R64G64B64_SFLOAT;
            case 4: return ChannelFormat::R64G64B64A64_SFLOAT;
            default: return ChannelFormat::R64_SFLOAT;
            }
        case TypeReflection::ScalarType::Float16:
            switch (elementCount)
            {
            case 1: return ChannelFormat::R16_SFLOAT;
            case 2: return ChannelFormat::R16G16_SFLOAT;
            case 3: return ChannelFormat::R16G16B16_SFLOAT;
            case 4: return ChannelFormat::R16G16B16A16_SFLOAT;
            default: return ChannelFormat::R16_SFLOAT;
            }
        default:
            ASSERT(false, "Unvalid scalar type");
        }

        return ChannelFormat::R8_UINT;
    }

    static Size GetScalarTypeSize(TypeReflection::ScalarType type)
    {
        switch (type)
        {
        case TypeReflection::ScalarType::None:
            return 0;
        case TypeReflection::ScalarType::Void:
            return 0;
        case TypeReflection::ScalarType::Bool:
            return 1;
        case TypeReflection::ScalarType::Int8:
        case TypeReflection::ScalarType::UInt8:
            return 1;
        case TypeReflection::ScalarType::Int16:
        case TypeReflection::ScalarType::UInt16:
            return 2;
        case TypeReflection::ScalarType::Int32:
        case TypeReflection::ScalarType::UInt32:
        case TypeReflection::ScalarType::Float32:
            return 4;
        case TypeReflection::ScalarType::Int64:
        case TypeReflection::ScalarType::UInt64:
        case TypeReflection::ScalarType::Float64:
            return 8;
        default:
            return 0;
        }
    }

    void ShaderObjectLayoutBuilder::ParseShaderProgramLayout(slang::ProgramLayout* programLayout,
                                                             ShaderCompilationObject& outCompilationObject)
    {
        m_indentation = 0;

        auto globalVarLayout = programLayout->getGlobalParamsVarLayout();

        auto entryPointCount = programLayout->getEntryPointCount();
        BlendState blendState;
        CullMode cullMode = CullMode::Back;
        BlendingMode blendMode = BlendingMode::Opaque;
        bool overridePipelineState = false;
        VkShaderStageFlags shaderStages = 0;

        for (int i = 0; i < entryPointCount; i++)
        {
            auto entryPoint = programLayout->getEntryPointByIndex(i);
            auto entryPointFunction = entryPoint->getFunction();
            if (auto shaderAttribute = entryPointFunction->findAttributeByName(m_globalSession, "shader"))
            {
                Size size = 0;
                auto shaderType = shaderAttribute->getArgumentValueString(0, &size);
                if (strcmp(shaderType, "fragment") == 0)
                    shaderStages |= VK_SHADER_STAGE_FRAGMENT_BIT;
                if (strcmp(shaderType, "geometry") == 0)
                    shaderStages |= VK_SHADER_STAGE_GEOMETRY_BIT;
                if (strcmp(shaderType, "vertex") == 0)
                {
                    shaderStages |= VK_SHADER_STAGE_VERTEX_BIT;
                    auto vertexInput = entryPointFunction->getParameterByIndex(0);
                    auto fields = vertexInput->getType()->getFieldCount();
                    for (auto f = 0; f < fields; f++)
                    {
                        auto field = vertexInput->getType()->getFieldByIndex(f);
                        auto name = field->getName();
                        auto type = field->getType()->getScalarType();
                        auto channelsCount = field->getType()->getElementCount();
                        auto fieldSize = GetScalarTypeSize(type);
                        LOG("Field {} Size {}", name, fieldSize);

                        VertexAttributeType vertexAttributeType = VertexAttributeType::UNDEFINED;

                        if (field->findAttributeByName(m_globalSession, "POSITION"))
                            vertexAttributeType = VertexAttributeType::POSITION;
                        else if (field->findAttributeByName(m_globalSession, "NORMAL"))
                            vertexAttributeType = VertexAttributeType::NORMAL;
                        else if (field->findAttributeByName(m_globalSession, "COLOR"))
                            vertexAttributeType = VertexAttributeType::COLOR;
                        else if (field->findAttributeByName(m_globalSession, "TANGENT"))
                            vertexAttributeType = VertexAttributeType::TANGENT;
                        else if (field->findAttributeByName(m_globalSession, "TEXCOORD0"))
                            vertexAttributeType = VertexAttributeType::TEXCOORD0;
                        else if (field->findAttributeByName(m_globalSession, "TEXCOORD1"))
                            vertexAttributeType = VertexAttributeType::TEXCOORD1;
                        else if (field->findAttributeByName(m_globalSession, "TEXCOORD2"))
                            vertexAttributeType = VertexAttributeType::TEXCOORD2;
                        else if (field->findAttributeByName(m_globalSession, "TEXCOORD3"))
                            vertexAttributeType = VertexAttributeType::TEXCOORD3;

                        outCompilationObject.m_vertexLayout.PushAttribute(
                            VertexAttribute(vertexAttributeType, GetChannelFormat(type, channelsCount)));
                    }
                }
            }

            if (auto blendingAttribute = entryPointFunction->findUserAttributeByName(m_globalSession, "Blending"))
            {
                int blendingModeInt = 0;
                blendingAttribute->getArgumentValueInt(0, &blendingModeInt);
                blendMode = static_cast<BlendingMode>(blendingModeInt);
                overridePipelineState = true;
            }
            if (const auto srcColorFactorAttribute = entryPointFunction->findUserAttributeByName(
                m_globalSession, "SrcColorFactor"))
            {
                int value = 0;
                srcColorFactorAttribute->getArgumentValueInt(0, &value);
                blendState.sourceColorFactor = static_cast<VkBlendFactor>(value);
                overridePipelineState = true;
                blendMode = BlendingMode::CustomBlend;
            }
            if (const auto dstColorFactorAttribute = entryPointFunction->findUserAttributeByName(
                m_globalSession, "DstColorFactor"))
            {
                int value = 0;
                dstColorFactorAttribute->getArgumentValueInt(0, &value);
                blendState.destinationColorFactor = static_cast<VkBlendFactor>(value);
                overridePipelineState = true;
                blendMode = BlendingMode::CustomBlend;
            }
            if (const auto colorBlendOpAttribute = entryPointFunction->findUserAttributeByName(
                m_globalSession, "ColorBlendOp"))
            {
                int value = 0;
                colorBlendOpAttribute->getArgumentValueInt(0, &value);
                blendState.colorBlendOperation = static_cast<VkBlendOp>(value);
                overridePipelineState = true;
                blendMode = BlendingMode::CustomBlend;
            }
            if (const auto alphaBlendOp = entryPointFunction->findUserAttributeByName(
                m_globalSession, "AlphaBlendOp"))
            {
                int value = 0;
                alphaBlendOp->getArgumentValueInt(0, &value);
                blendState.alphaBlendOperation = static_cast<VkBlendOp>(value);
                overridePipelineState = true;
                blendMode = BlendingMode::CustomBlend;
            }
            if (const auto cullModeAttribute = entryPointFunction->findUserAttributeByName(
                m_globalSession, "Culling"))
            {
                int cullModeInt = 0;
                cullModeAttribute->getArgumentValueInt(0, &cullModeInt);
                cullMode = static_cast<CullMode>(cullModeInt);
                overridePipelineState = true;
            }
            if (const auto colorMaskAttribute = entryPointFunction->findUserAttributeByName(
                m_globalSession, "ColorMask"))
            {
                int maskValue = 0;
                colorMaskAttribute->getArgumentValueInt(0, &maskValue);
                blendState.colorMask = static_cast<ColorMask::Enum>(maskValue);
                overridePipelineState = true;
            }
        }

        outCompilationObject.m_overridesPipelineState = overridePipelineState;
        outCompilationObject.m_cullMode = cullMode;

        switch (blendMode)
        {
        case BlendingMode::Opaque:
            blendState.colorBlendOperation = VK_BLEND_OP_ADD;
            blendState.alphaBlendOperation = VK_BLEND_OP_ADD;
            blendState.sourceColorFactor = VK_BLEND_FACTOR_ONE;
            blendState.destinationColorFactor = VK_BLEND_FACTOR_ZERO;
            blendState.sourceAlphaFactor = VK_BLEND_FACTOR_ONE;
            blendState.destinationAlphaFactor = VK_BLEND_FACTOR_ZERO;
            break;
        case BlendingMode::AdditiveBlend:
            blendState.colorBlendOperation = VK_BLEND_OP_ADD;
            blendState.alphaBlendOperation = VK_BLEND_OP_ADD;
            blendState.sourceColorFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendState.destinationColorFactor = VK_BLEND_FACTOR_ONE;
            blendState.sourceAlphaFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendState.destinationAlphaFactor = VK_BLEND_FACTOR_ONE;
            break;
        case BlendingMode::SubtractiveBlend:
            blendState.colorBlendOperation = VK_BLEND_OP_SUBTRACT;
            blendState.alphaBlendOperation = VK_BLEND_OP_SUBTRACT;
            blendState.sourceColorFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendState.destinationColorFactor = VK_BLEND_FACTOR_ONE;
            blendState.sourceAlphaFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendState.destinationAlphaFactor = VK_BLEND_FACTOR_ONE;
            break;
        case BlendingMode::MultiplyBlend:
            blendState.colorBlendOperation = VK_BLEND_OP_MULTIPLY_EXT;
            blendState.alphaBlendOperation = VK_BLEND_OP_MULTIPLY_EXT;
            blendState.sourceColorFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendState.destinationColorFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            blendState.sourceAlphaFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendState.destinationAlphaFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;
        case BlendingMode::AlphaBlend:
            blendState.colorBlendOperation = VK_BLEND_OP_ADD;
            blendState.alphaBlendOperation = VK_BLEND_OP_ADD;
            blendState.sourceColorFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendState.destinationColorFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            blendState.sourceAlphaFactor = VK_BLEND_FACTOR_ONE;
            blendState.destinationAlphaFactor = VK_BLEND_FACTOR_ZERO;
        default:
            break;
        }

        if (blendMode == BlendingMode::CustomBlend || overridePipelineState)
            outCompilationObject.m_blendState = blendState;

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

        for (Size i = 0; i < context.GetConstantBufferCount(); i++)
        {
            auto& cbuffer = context.GetConstantBufferInfo(i);
            if (cbuffer.size > 0)
            {
                buffersToAllocate.push_back(cbuffer);
            }
        }

        outCompilationObject.descriptorsData.setsCount = context.getSetIndex() + 1;

        outCompilationObject.SetPushConstants(context.GetPushConstants(), context.GetPushConstantsCount());
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
        PushConstantInfo pushConstants{};
        for (Size i = 0; i < context.GetPushConstantsCount(); i++)
        {
            auto& pushBuffer = context.GetPushConstantsInfo(i);
            pushBuffer.shaderStages = shaderStages;
            VkPushConstantRange pushConstantRange;
            pushConstantRange.offset = pushBuffer.offset;
            pushConstantRange.stageFlags = pushBuffer.shaderStages;
            pushConstantRange.size = pushBuffer.size;

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

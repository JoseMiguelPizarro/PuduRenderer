//
// Created by Hojaverde on 2/23/2025.
//

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
        case slang::BindingType::PushConstant:
        default:
            std::fprintf(stderr, "Assertion failed: %s %d\n", "Unsupported binding type!", slangBindingType);
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
                                       : "Unnamed";
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

    void ShaderObjectLayoutBuilder::AddBindingsForParameterBlock(slang::TypeLayoutReflection* typeLayout,
                                                                 DescriptorSetLayoutsData& layoutsData)
    {
        auto fieldCount = typeLayout->getFieldCount();
        u16 setsCount = 0;


        for (size i = 0; i < fieldCount; i++)
        {
            auto field = typeLayout->getFieldByIndex(i);
            auto fieldLayout = field->getTypeLayout()->getElementTypeLayout();

            auto containerLayout = field->getTypeLayout()->getContainerVarLayout();


            LOG_I(m_indentation, "Shader field: {} category: {}", field->getName(),
                  (u32)fieldLayout->getParameterCategory());

            size descriptorSet = field->getOffset(SLANG_PARAMETER_CATEGORY_SUB_ELEMENT_REGISTER_SPACE);

            slang::BindingType type = fieldLayout->getBindingRangeType(0);
            if (type == slang::BindingType::PushConstant)
            {
                LOG_I(m_indentation, "Push constants! {} set {} binding {}", field->getName(), descriptorSet, 0);
            }

            if (field->getType()->getKind() == TypeReflection::Kind::ConstantBuffer)
            {
                LOG_I(m_indentation, "Constant buffer layout not supported {}", field->getName());
                //We'll only process parameterblocks
                continue;
            }

            auto propertiesCount = fieldLayout->getFieldCount();

            std::vector<DescriptorSetLayoutData*> layoutsPtr;

            for (size_t i = 0; i < layoutsData.layoutData.size(); i++)
            {
                layoutsPtr.push_back(&layoutsData.layoutData[i]);
            }

            DescriptorSetLayoutData layoutData{};


            bool layoutExists = boolinq::from(layoutsPtr).any([descriptorSet](const DescriptorSetLayoutData* l)
            {
                return l->SetNumber == descriptorSet;
            });

            auto bindless = field->getVariable()->findUserAttributeByName(m_globalSession, "Bindless");
            bool isBindless = false;

            if (bindless)
            {
                int v;
                bindless->getArgumentValueInt(0, &v);
                isBindless = bindless;
            }

            for (size_t j = 0; j < propertiesCount; j++)
            {
                auto property = fieldLayout->getFieldByIndex(j);

                uint16_t bindingCount = 1;

                if (property->getType()->getKind() == TypeReflection::Kind::Array)
                {
                    bindingCount = property->getTypeLayout()->getElementCount();
                }

                DescriptorBinding binding;

                auto descriptorType = ToVk(property->getTypeLayout()->getBindingRangeType(0));
                binding.set = static_cast<uint16_t>(descriptorSet);
                binding.index = property->getBindingIndex();
                binding.type = descriptorType;
                binding.count = bindingCount;
                binding.name = std::format("{}.{}", field->getName(), property->getName());

                layoutsData.bindingsData.push_back(binding);

                LOG_I(m_indentation, "Shader property: {}", property->getName());


                VkDescriptorSetLayoutBinding layoutBinding = {};
                layoutBinding.binding = binding.index;
                layoutBinding.descriptorType = descriptorType;
                layoutBinding.descriptorCount = bindingCount;
                layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT |
                    VK_SHADER_STAGE_COMPUTE_BIT; //TODO: Hack, but in all stages for now
                layoutBinding.pImmutableSamplers = nullptr;
                layoutData.Bindings.push_back(layoutBinding);
            }
            if (!layoutExists)
            {
                layoutData.SetNumber = descriptorSet;
                layoutData.CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutData.CreateInfo.bindingCount = layoutData.Bindings.size();
                //We might have issues here if the binding count doesn't consider the ones in vertex/fragment
                layoutData.CreateInfo.pBindings = layoutData.Bindings.data();
                layoutData.name = field->getName();
                layoutData.bindless = isBindless;
                layoutsData.layoutData.push_back(layoutData);

                setsCount++;
            }
        }

        layoutsData.setsCount = setsCount;
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
    }

    CumulativeOffset calculateCumulativeOffset(
        slang::ParameterCategory layoutUnit,
        AccessPath accessPath)
    {
        CumulativeOffset result;
        switch (layoutUnit)
        {
        // #### Layout Units That Don't Require Special Handling
        //
        default:
            for (auto node = accessPath.leaf; node != nullptr; node = node->parent)
            {
                result.value += node->variableLayout->getOffset(layoutUnit);
            }
            break;

        // #### Bytes
        //
        case slang::ParameterCategory::Uniform:
            for (auto node = accessPath.leaf; node != accessPath.deepestConstantBufer;
                 node = node->parent)
            {
                result.value += node->variableLayout->getOffset(layoutUnit);
            }
            break;

        // #### Layout Units That Care About Spaces
        //
        case slang::ParameterCategory::ConstantBuffer:
        case slang::ParameterCategory::ShaderResource:
        case slang::ParameterCategory::UnorderedAccess:
        case slang::ParameterCategory::SamplerState:
        case slang::ParameterCategory::DescriptorTableSlot:
            for (auto node = accessPath.leaf; node != accessPath.deepestParameterBlock;
                 node = node->parent)
            {
                result.value += node->variableLayout->getOffset(layoutUnit);
                result.space += node->variableLayout->getOffset(SLANG_PARAMETER_CATEGORY_SUB_ELEMENT_REGISTER_SPACE);
            }
            for (auto node = accessPath.deepestParameterBlock; node != nullptr; node = node->parent)
            {
                result.space += node->variableLayout->getOffset(
                    SLANG_PARAMETER_CATEGORY_SUB_ELEMENT_REGISTER_SPACE);
            }
            break;
        }

        return result;
    }

    CumulativeOffset ShaderObjectLayoutBuilder::CalculateCumulativeOffset(
        slang::VariableLayoutReflection* variableLayout, slang::ParameterCategory layoutUnit, AccessPath accessPath)
    {
        CumulativeOffset result = calculateCumulativeOffset(layoutUnit, accessPath);
        result.value += variableLayout->getOffset(layoutUnit);
        result.space += variableLayout->getBindingSpace(layoutUnit);
        return result;
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
                innerOffsets.deepestConstantBufer = innerOffsets.leaf;
                if (containerVarLayout->getTypeLayout()->getSize(
                    slang::ParameterCategory::SubElementRegisterSpace) != 0)
                {
                    innerOffsets.deepestParameterBlock = innerOffsets.leaf;
                }

                accessPath.rootBufferInfo = context->PushConstantBufferInfo();
                if (kind == TypeReflection::Kind::ParameterBlock )
                {
                    context->setIndex++;
                    accessPath.setIndex = context->setIndex;

                }

                ExtendedAccessPath elementOffsets(innerOffsets, elementVarLayout);
                ParseVariableOffsets(containerVarLayout, context, accessPath);
                ParseVariableTypeLayout(elementVarLayout->getTypeLayout(), context, accessPath);
            }
            break;

            ///Here we should push a binding
        case TypeReflection::Kind::Resource:
            {

                auto element = typeLayoutReflection->getElementVarLayout();

                auto offset = CalculateCumulativeOffset(element, SubElementRegisterSpace, accessPath);
                LOG_I(m_indentation, "{} Set: {} Index: {}", element->getName() ? element->getName() : "", accessPath.setIndex,
                      typeLayoutReflection->getBindingRangeCount());
                auto descriptorType = string_VkDescriptorType(ToVk(typeLayoutReflection->getBindingRangeType(0)));
                LOG_I(m_indentation, "Descriptor type: {}", descriptorType);
                ParseVariableOffsets(typeLayoutReflection->getElementVarLayout(), context, accessPath);
            }
            break;
            //Here we should push a binding
        case TypeReflection::Kind::Scalar:
        case TypeReflection::Kind::Vector:
        case TypeReflection::Kind::Matrix:
            {
                DescriptorBinding binding;

                auto container = typeLayoutReflection->getContainerVarLayout();
                LOG_I(m_indentation, "Container: {}",KIND_NAMES.at(container->getType()->getKind()));

                auto descriptorType = string_VkDescriptorType(ToVk(typeLayoutReflection->getBindingRangeType(0)));
                LOG_I(m_indentation, "Size: {} Binding: {}", accessPath.setIndex, typeLayoutReflection->getBindingRangeCount());
                LOG_I(m_indentation, "Descriptor type: {}", descriptorType);
            }
            break;
            case TypeReflection::Kind::Array:
                {
                  LOG_I(m_indentation, "Array ElementCount: {} Size:", typeLayoutReflection->getElementCount(), typeLayoutReflection->getSize());
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

        ParseVariableTypeLayout(varLayout->getTypeLayout(), context, static_cast<AccessPath>(varPath));
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
            case SubElementRegisterSpace:
                {
                    auto [value, space] = CalculateCumulativeOffset(varLayout, category, accessPath);
                    LOG_I(m_indentation, " Set: {} value: {}", accessPath.setIndex, value);
                }
                break;
            case Uniform:
                {
                    auto [value, space] = CalculateCumulativeOffset(varLayout, category, accessPath);
                    LOG_I(m_indentation, "Size:{} Set: {} value: {}", varLayout->getTypeLayout()->getSize(), accessPath.setIndex,
                          value);

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
                    auto offset = CalculateCumulativeOffset(varLayout, category, accessPath);
                    auto bufferSize = varLayout->getTypeLayout()->getSize(category);
                    LOG_I(m_indentation, "set: {} value: {}", accessPath.setIndex, varLayout->getBindingIndex());
                }
                break;
            default: break;
            }
        }
        m_indentation--;
    }


    void ShaderObjectLayoutBuilder::ParseScope(slang::VariableLayoutReflection* scopeVarLayout,
                                               ShaderLayoutBuilderContext* context, AccessPath accessPath)
    {
        m_indentation++;
        ExtendedAccessPath scopeOffsets(accessPath, scopeVarLayout);

        TypeLayoutReflection* scopeTypeLayout = scopeVarLayout->getTypeLayout();
        ConstantBufferInfo rootConstantBufferInfo;

        scopeOffsets.rootBufferInfo = &rootConstantBufferInfo;

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
                ParseVariableOffsets(containerLayout, context, static_cast<AccessPath>(scopeOffsets));
                ParseScope(scopeTypeLayout->getElementVarLayout(), context, static_cast<AccessPath>(scopeOffsets));
                ConstantBufferInfo constantBufferInfo;
            }
            break;

        case TypeReflection::Kind::ParameterBlock:
            LOG_I(m_indentation, "Parsing ParameterBlock");
            ParseVariableOffsets(scopeTypeLayout->getElementVarLayout(), context, static_cast<AccessPath>(scopeOffsets));
            ParseScope(scopeTypeLayout->getElementVarLayout(), context, static_cast<AccessPath>(scopeOffsets));
            break;
        default:
            break;
        }
        m_indentation--;
    }
} // Pudu

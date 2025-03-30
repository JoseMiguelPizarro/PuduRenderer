#include "Material.h"
#include "PuduGraphics.h"
#include "PuduConstants.h"
#include "ShaderCursor/ShaderCursor.h"

namespace Pudu
{
    VkPipelineBindPoint GetBindingPoint(Pipeline* pipeline)
    {
        switch (pipeline->pipelineType)
        {
        case PipelineType::Compute:
            return VK_PIPELINE_BIND_POINT_COMPUTE;
        case PipelineType::Graphics:
            return VK_PIPELINE_BIND_POINT_GRAPHICS;
        default:
            break;
        }

        throw std::invalid_argument("Invalid pipeline type");
    }

    void Material::ApplyProperties()
    {
        MaterialApplyPropertyGPUTarget target;
        target.graphics = m_gpu;
        target.descriptorProvider = m_descriptorProvider.get();
        target.m_descriptorSetRemap = m_descriptorSetsIndexRemap;
        target.m_descriptorSets = m_descriptorSets;

        m_propertiesBlock.ApplyProperties(target);
    }

    void Material::SetProperty(const std::string_view& name, const float value)
    {
        m_propertiesBlock.SetProperty(name, value);
    }

    void Material::SetProperty(const std::string& name, const glm::vec2 value)
    {
        m_propertiesBlock.SetProperty(name, value);
    }

    void Material::SetProperty(const std::string& name, const SPtr<Texture>& texture)
    {
        m_propertiesBlock.SetProperty(name, texture);
    }

    void Material::SetProperty(const std::string& name, const SPtr<GraphicsBuffer>& buffer)
    {
        m_propertiesBlock.SetProperty(name, buffer);
    }

    void Material::SetProperty(const std::string& name, std::vector<SPtr<Texture>>* textureArray)
    {
        m_propertiesBlock.SetProperty(name, textureArray);
    }

    Material* Material::SetScope(const char* scope)
    {
        ASSERT(m_descriptorProvider == nullptr, "Material scope should be set before setting descriptor provider");

        m_scope = scope;

        return this;
    }

    Size Material::GetDescriptorSetsCount() const
    {
        return m_descriptorSetCount;
    }

    void Material::AllocateDescriptorSetsResources(const SPtr<IDescriptorProvider>& descriptorProvider)
    {
        if (m_resourcesAllocated)
        {
            LOG_WARNING("Trying to allocate descriptor sets resources while already allocated");
            return;
        }

        auto allocationInfo = ShaderNodeResourcesAllocationInfo{
            descriptorProvider->GetShaderLayout(), m_gpu, m_scope.c_str()
        };

        m_propertiesBlock.AllocateGPUResourcesFromShaderNode(allocationInfo);

        m_resourcesAllocated = true;
    }

    VkDescriptorSet MaterialApplyPropertyGPUTarget::GetDescriptorSet(const Size slotIndex) const
    {
        if (m_descriptorSetRemap == nullptr)
        {
            return m_descriptorSets[slotIndex];
        }

        return m_descriptorSets[m_descriptorSetRemap[slotIndex]];
    }

    void ShaderPropertiesBlock::SetProperty(const std::string_view& name, float value)
    {
        PropertyUpdateRequest request;
        request.property.name = name;
        request.property.type = ShaderPropertyType::Float;
        request.property.value = glm::vec4(value);

        m_descriptorUpdateRequests.push_back(request);
    }

    void ShaderPropertiesBlock::SetProperty(const std::string& name, const glm::vec2 value)
    {
        PropertyUpdateRequest request;
        request.property.name = name;
        request.property.type = ShaderPropertyType::Vec2;
        request.property.value = glm::vec4(value.x, value.y, 0.0f, 0.0f);

        m_descriptorUpdateRequests.push_back(request);
    }

    void ShaderPropertiesBlock::SetProperty(const std::string& name, const SPtr<Texture>& texture)
    {
        PropertyUpdateRequest updateRequest{};
        updateRequest.property.texture = texture;
        updateRequest.property.name = name;
        updateRequest.property.type = ShaderPropertyType::Texture;

        m_descriptorUpdateRequests.push_back(updateRequest);
    }

    void ShaderPropertiesBlock::SetProperty(const std::string& name, const SPtr<GraphicsBuffer>& buffer)
    {
        PropertyUpdateRequest updateRequest{};
        updateRequest.property.type = ShaderPropertyType::Buffer;
        updateRequest.property.name = name;
        updateRequest.property.buffer = buffer;

        m_descriptorUpdateRequests.push_back(updateRequest);
    }

    void ShaderPropertiesBlock::SetProperty(const std::string& name, std::vector<SPtr<Texture>>* textureArray)
    {
        PropertyUpdateRequest updateRequest{};
        updateRequest.property.type = ShaderPropertyType::TextureArray;
        updateRequest.property.name = name;
        updateRequest.property.textureArray = textureArray;

        m_descriptorUpdateRequests.push_back(updateRequest);
    }

    void ShaderPropertiesBlock::ApplyProperties(const MaterialApplyPropertyGPUTarget& target)
    {
        for (auto& request : m_descriptorUpdateRequests)
        {
            switch (request.property.type)
            {
            case ShaderPropertyType::Texture:
                {
                    ApplyTexture(request, target);
                    break;
                }
            case ShaderPropertyType::Buffer:
                {
                    ApplyBuffer(request, target);
                    break;
                }
            case ShaderPropertyType::TextureArray:
                {
                    ApplyTextureArray(request, target);
                }
                break;
            case ShaderPropertyType::Vec2:
                {
                    ApplyVectorValue(request, target);
                }
                break;
            case ShaderPropertyType::Float:
                {
                    ApplyFloatValue(request, target);
                }
                break;
            default:
                break;
            }
        }

        m_descriptorUpdateRequests.clear();
    }

    std::vector<SPtr<GPUResourceBase>>* ShaderPropertiesBlock::GetAllocatedResources()
    {
        return &m_allocatedResources;
    }

    void ShaderPropertiesBlock::ApplyTexture(PropertyUpdateRequest& request,
                                             const MaterialApplyPropertyGPUTarget& target)
    {
        auto shaderCursor = ShaderCursor(target.descriptorProvider->GetShaderLayout(), &target);

        auto field = shaderCursor.Field(request.property.name.c_str());

        if (field.IsValid())
        {
            field.Write(request.property.texture);
            BindPropertyToShaderNode(field.GetNode(), request.property);
        }
        else
        {
            LOG_WARNING("Texture {} not found", request.property.name.c_str());
        }
    }

    void ShaderPropertiesBlock::ApplyBuffer(PropertyUpdateRequest& request,
                                            const MaterialApplyPropertyGPUTarget& target)
    {
        auto shaderCursor = ShaderCursor(target.descriptorProvider->GetShaderLayout(), &target);

        auto field = shaderCursor.Field(request.property.name.c_str());

        if (field.IsValid())
        {
            field.Write(request.property.buffer);
            BindPropertyToShaderNode(field.GetNode(), request.property);
        }
        else
            LOG_WARNING("Buffer {} not found", request.property.name.c_str());
    }

    void ShaderPropertiesBlock::ApplyTextureArray(PropertyUpdateRequest& request,
                                                  const MaterialApplyPropertyGPUTarget& target)
    {
        auto shaderCursor = ShaderCursor(target.descriptorProvider->GetShaderLayout(), &target);
        auto field = shaderCursor.Field(request.property.name.c_str());

        LOG_ERROR("ApplyTextureArray NOT IMPLEMENTED");

        //TODO: HANDLE TEXTURE ARRAY LOGIC

        auto binding = target.descriptorProvider->GetBindingByName(request.property.name.c_str());

        if (binding == nullptr)
        {
            LOG("Trying to set non-existing parameter {} for descriptor provider", request.property.name);
            return;
        }

        static VkWriteDescriptorSet descriptorWrites[PuduGraphics::k_MAX_BINDLESS_RESOURCES];
        static VkDescriptorImageInfo imageInfos[PuduGraphics::k_MAX_BINDLESS_RESOURCES];

        auto textureArray = request.property.textureArray;
        uint32_t currentWriteIndex = 0;

        for (const auto& texture : *textureArray)
        {
            VkWriteDescriptorSet& descriptorWrite = descriptorWrites[currentWriteIndex];
            descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.dstArrayElement = texture->Handle();
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.dstSet = target.GetDescriptorSet(binding->setNumber);

            descriptorWrite.dstBinding = binding->index;

            auto textureSampler = texture->Sampler;

            VkDescriptorImageInfo& descriptorImageInfo = imageInfos[currentWriteIndex];
            descriptorImageInfo.sampler = textureSampler.vkHandle;
            descriptorImageInfo.imageView = texture->vkImageViewHandle;
            descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            descriptorWrite.pImageInfo = &descriptorImageInfo;

            currentWriteIndex++;
        }

        target.graphics->UpdateDescriptorSet(currentWriteIndex, descriptorWrites);
        //settings.commands->PushDescriptorSets(GetBindingPoint(settings.pipeline), settings.pipeline->vkPipelineLayoutHandle, setIndex, currentWriteIndex, descriptorWrites);
    }

    void ShaderPropertiesBlock::ApplyVectorValue(const PropertyUpdateRequest& request,
                                                 const MaterialApplyPropertyGPUTarget& target)
    {
        auto shaderCursor = ShaderCursor(target.descriptorProvider->GetShaderLayout(), &target);
        auto field = shaderCursor.Field(request.property.name.c_str());

        if (field.IsValid())
        {
            auto shaderNode = field.GetNode();

            //Fetch float value associated buffer
            auto cBufferProperty = FetchShaderNodeProperty(shaderNode->parentContainer);

            if (cBufferProperty != nullptr)
            {
                field.Write(cBufferProperty->buffer, &request.property.value, shaderNode->offset, shaderNode->size);
                BindPropertyToShaderNode(field.GetNode(), request.property);
            }
            else
            {
                LOG_WARNING("Trying to set vector {} value but parent cbuffer has not been bound",
                            shaderNode->GetFullPath());
            }
        }
        else
        {
            LOG_WARNING("Shader property {} not found", request.property.name.c_str());
        }
    }

    void ShaderPropertiesBlock::ApplyFloatValue(const PropertyUpdateRequest& request,
                                                const MaterialApplyPropertyGPUTarget& target)
    {
        //For alignment purposes we are treating everything as a vector hehe 🙈
        ApplyVectorValue(request, target);
    }

    void ShaderPropertiesBlock::AllocateGPUResourcesFromShaderNode(ShaderNodeResourcesAllocationInfo& allocationInfo)
    {
        auto node = allocationInfo.rootNode;

        ASSERT(node != nullptr, "Shader node can't be null!");

        //TODO: CHECK THAT THE BUFFER IS BOTH IN SCOPE AND THAT IS AN AUTOMATICALLY ADDED CBUFFER
        if (node->type == ShaderNodeType::CBuffer)
        {
            if (node->scope == allocationInfo.scope)
            {
                auto cbuffer =
                    allocationInfo.graphics->CreateGraphicsBuffer(node->size, nullptr,
                                                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                                                  |
                                                                  VMA_ALLOCATION_CREATE_MAPPED_BIT,
                                                                  node->name.c_str());

                m_allocatedResources.push_back(cbuffer);

                ShaderProperty shaderProperty{};
                shaderProperty.type = ShaderPropertyType::Buffer;
                shaderProperty.name = node->GetFullPath();
                shaderProperty.buffer = cbuffer;

                SetProperty(shaderProperty.name, shaderProperty.buffer);
                BindPropertyToShaderNode(node, shaderProperty);
            }
        }

        for (Size i = 0; i < node->childCount; i++)
        {
            const auto childNode = node->GetChildByIndex(i);

            auto allocInfo = ShaderNodeResourcesAllocationInfo(
                childNode, allocationInfo.graphics, allocationInfo.scope);

            AllocateGPUResourcesFromShaderNode(allocInfo);
        }
    }

    void ShaderPropertiesBlock::BindPropertyToShaderNode(ShaderNode* node, const ShaderProperty& property)
    {
        m_propertiesByShaderNodeMap[node] = property;
    }

    ShaderProperty* ShaderPropertiesBlock::FetchShaderNodeProperty(const ShaderNode* node)
    {
        if (m_propertiesByShaderNodeMap.contains(node))
        {
            return &m_propertiesByShaderNodeMap[node];
        }

        LOG_WARNING("TRYING TO FETCH SHADER PROPERTY NOT PREVIOUSLY SET {}", node->GetFullPath());

        return nullptr;
    }

    Material::Material(PuduGraphics* graphics)
    {
        this->Create(graphics);
    }

    void Material::SetShader(const SPtr<Shader>& shader)
    {
        m_shader = shader;

        SetDescriptorProvider(std::dynamic_pointer_cast<IDescriptorProvider>(shader));
    }

    void Material::SetDescriptorProvider(const SPtr<IDescriptorProvider>& descriptorProvider)
    {
        ASSERT(m_descriptorProvider == nullptr, "Descriptor provider already set");

        m_descriptorProvider = descriptorProvider;

        std::vector<VkDescriptorSetLayout> descriptorSetsToCreate;
        descriptorSetsToCreate.reserve(m_descriptorProvider->GetDescriptorSetLayouts()->size());

        auto descriptorSetLayouts = m_descriptorProvider->GetDescriptorSetLayouts();

        Size descriptorSetCount = 0;
        for (const auto& layout : *descriptorSetLayouts)
        {
            if (layout->scope == m_scope)
            {
                m_descriptorSetsIndexRemap[layout->setIndex] = descriptorSetCount++;
                descriptorSetsToCreate.push_back(layout->vkHandle);
            }
        }

        m_descriptorSetCount = descriptorSetCount;
        //TODO: STORE A SHADER ROOT NODE THAT PARENTS ALL THE CHILD SHADERNODES? Since set number is cached on the shaderNode itself, this will need recompute it each time or recache it

        m_gpu->CreateDescriptorSets(m_descriptorSets, descriptorSetsToCreate.size(),
                                    descriptorSetsToCreate.data());

        auto allocationInfo = ShaderNodeResourcesAllocationInfo(descriptorProvider->GetShaderLayout(), m_gpu,
                                                                m_scope.c_str());
        m_propertiesBlock.AllocateGPUResourcesFromShaderNode(allocationInfo);
    }
}

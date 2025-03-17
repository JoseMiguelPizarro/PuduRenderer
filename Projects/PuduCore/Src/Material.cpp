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
        m_propertiesBlock.ApplyProperties({m_gpu, m_descriptorProvider.get(), m_descriptorSets});
    }

    void Material::SetProperty(const std::string_view& name, const float value)
    {
        m_propertiesBlock.SetProperty(name, value);
    }

    void Material::SetProperty(const std::string& name, const glm::vec2 value)
    {
        m_propertiesBlock.SetProperty(name, value);
    }

    void Material::SetProperty(const std::string& name, const SPtr<Pudu::Texture>& texture)
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

    size Material::GetDescriptorSetsCount() const
    {
        return m_descriptorProvider->GetDescriptorSetLayouts()->size();
    }

    void ShaderPropertiesBlock::SetProperty(const std::string_view& name, float value)
    {
        PropertyUpdateRequest request;
        request.name = name;
        request.type = ShaderPropertyType::Float;
        request.value = glm::vec4(value);

        m_descriptorUpdateRequests.push_back(request);
    }

    void ShaderPropertiesBlock::SetProperty(const std::string& name, const glm::vec2 value)
    {
        PropertyUpdateRequest request;
        request.name = name;
        request.type = ShaderPropertyType::Vec2;
        request.value = glm::vec4(value.x, value.y, 0.0f, 0.0f);

        m_descriptorUpdateRequests.push_back(request);
    }

    void ShaderPropertiesBlock::SetProperty(const std::string& name, const SPtr<Texture>& texture)
    {
        PropertyUpdateRequest updateRequest{};
        updateRequest.texture = texture;
        updateRequest.name = name;
        updateRequest.type = ShaderPropertyType::Texture;

        m_descriptorUpdateRequests.push_back(updateRequest);
    }

    void ShaderPropertiesBlock::SetProperty(const std::string& name, const SPtr<GraphicsBuffer>& buffer)
    {
        PropertyUpdateRequest updateRequest{};
        updateRequest.type = ShaderPropertyType::Buffer;
        updateRequest.name = name;
        updateRequest.buffer = buffer;

        m_descriptorUpdateRequests.push_back(updateRequest);
    }

    void ShaderPropertiesBlock::SetProperty(const std::string& name, std::vector<SPtr<Texture>>* textureArray)
    {
        PropertyUpdateRequest updateRequest{};
        updateRequest.type = ShaderPropertyType::TextureArray;
        updateRequest.name = name;
        updateRequest.textureArray = textureArray;

        m_descriptorUpdateRequests.push_back(updateRequest);
    }

    void ShaderPropertiesBlock::ApplyProperties(const MaterialApplyPropertyGPUTarget& target)
    {
        for (auto& request : m_descriptorUpdateRequests)
        {
            switch (request.type)
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
                ApplyTextureArray(request, target);
                break;
            case ShaderPropertyType::Vec2:
                ApplyVectorValue(request, target);
                break;
            case ShaderPropertyType::Float:
                ApplyFloatValue(request, target);
                break;
            default:
                break;
            }
        }

        m_descriptorUpdateRequests.clear();
    }

    void ShaderPropertiesBlock::ApplyTexture(PropertyUpdateRequest& request,
                                             const MaterialApplyPropertyGPUTarget& target)
    {
        auto shaderCursor = ShaderCursor(target.descriptorProvider->GetShaderLayout(), &target);

        shaderCursor.Field(request.name.c_str()).Write(request.texture);
    }

    void ShaderPropertiesBlock::ApplyBuffer(PropertyUpdateRequest& request,
                                            const MaterialApplyPropertyGPUTarget& target)
    {
        auto shaderCursor = ShaderCursor(target.descriptorProvider->GetShaderLayout(), &target);

        shaderCursor.Field(request.name.c_str()).Write(request.buffer);
        //	target.commands->PushDescriptorSets(GetBindingPoint(target.pipeline), target.pipeline->vkPipelineLayoutHandle, binding->set, 1, &bufferWrite);
    }

    void ShaderPropertiesBlock::ApplyTextureArray(PropertyUpdateRequest& request,
                                                  const MaterialApplyPropertyGPUTarget& settings)
    {
        auto binding = settings.descriptorProvider->GetBindingByName(request.name.c_str());

        if (binding == nullptr)
        {
            LOG("Trying to set non-existing parameter {} for descriptor provider", request.name);
            return;
        }

        static VkWriteDescriptorSet descriptorWrites[PuduGraphics::k_MAX_BINDLESS_RESOURCES];
        static VkDescriptorImageInfo imageInfos[PuduGraphics::k_MAX_BINDLESS_RESOURCES];

        auto textureArray = request.textureArray;
        uint32_t currentWriteIndex = 0;

        for (const auto& texture : *textureArray)
        {
            VkWriteDescriptorSet& descriptorWrite = descriptorWrites[currentWriteIndex];
            descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.dstArrayElement = texture->Handle();
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.dstSet = settings.descriptorSets[binding->setNumber];

            descriptorWrite.dstBinding = binding->index;

            auto textureSampler = texture->Sampler;

            VkDescriptorImageInfo& descriptorImageInfo = imageInfos[currentWriteIndex];
            descriptorImageInfo.sampler = textureSampler.vkHandle;
            descriptorImageInfo.imageView = texture->vkImageViewHandle;
            descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            descriptorWrite.pImageInfo = &descriptorImageInfo;

            currentWriteIndex++;
        }

        settings.graphics->UpdateDescriptorSet(currentWriteIndex, descriptorWrites);
        //settings.commands->PushDescriptorSets(GetBindingPoint(settings.pipeline), settings.pipeline->vkPipelineLayoutHandle, setIndex, currentWriteIndex, descriptorWrites);
    }

    void ShaderPropertiesBlock::ApplyVectorValue(PropertyUpdateRequest& request,
                                                 const MaterialApplyPropertyGPUTarget& settings)
    {
        throw std::runtime_error("Unimplemented ApplyVectorValue");
        auto binding = settings.descriptorProvider->GetBindingByName(request.name.c_str());

        if (binding == nullptr)
        {
            LOG("Trying to set non-existing parameter {} for descriptor provider", request.name);
            return;
        }

        VkWriteDescriptorSet bufferWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        bufferWrite.descriptorCount = 1;
        bufferWrite.dstBinding = binding->index;
        bufferWrite.dstSet = settings.descriptorSets[binding->setNumber];
        // bufferWrite.pBufferInfo = &bufferInfo;

        bufferWrite.descriptorType = binding->type;

        // target.graphics->UpdateDescriptorSet(1, &bufferWrite);
    }

    void ShaderPropertiesBlock::ApplyFloatValue(const PropertyUpdateRequest& value,
                                                const MaterialApplyPropertyGPUTarget& target)
    {
    }

    Material::Material(PuduGraphics* graphics)
    {
        this->Create(graphics);
    }

    void Material::SetShader(SPtr<Shader> shader)
    {
        m_shader = shader;
        const auto layouts = shader->GetVkDescriptorSetLayouts();
        const auto layoutCount = shader->GetActiveLayoutCount();

        m_gpu->CreateDescriptorSets(m_descriptorSets, layoutCount, layouts);

        m_descriptorProvider = std::dynamic_pointer_cast<IDescriptorProvider>(shader);
    }

    void Material::SetDescriptorProvider(const SPtr<IDescriptorProvider>& descriptorProvider)
    {
        m_descriptorProvider = descriptorProvider;

        m_gpu->CreateDescriptorSets(m_descriptorSets, m_descriptorProvider->GetDescriptorSetLayouts()->size(),
                                    m_descriptorProvider->GetVkDescriptorSetLayouts());
    }

    void Material::CreateDescriptorSets(const std::vector<SPtr<DescriptorSetLayout>>& layouts)
    {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts(layouts.size());
        for (uint32_t i = 0; i < layouts.size(); i++)
        {
            descriptorSetLayouts[i] = layouts[i]->vkHandle;
        }
        m_gpu->CreateDescriptorSets(m_descriptorSets, layouts.size(), descriptorSetLayouts.data());
    }
}

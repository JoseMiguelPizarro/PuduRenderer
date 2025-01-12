#include "Material.h"
#include "PuduGraphics.h"
#include "PuduConstants.h"

namespace Pudu {

	VkPipelineBindPoint GetBindingPoint(Pipeline* pipeline) {

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

	void Material::SetProperty(std::string name, SPtr<Pudu::Texture> texture)
	{
		m_propertiesBlock.SetProperty(name, texture);
	}

	void Material::SetProperty(std::string name, SPtr<GraphicsBuffer> buffer) {
		m_propertiesBlock.SetProperty(name, buffer);
	}

	void Material::SetProperty(std::string name, std::vector<SPtr<Texture>>* textureArray)
	{
		m_propertiesBlock.SetProperty(name, textureArray);
	}

	void ShaderPropertiesBlock::SetProperty(std::string name, SPtr<Texture> texture)
	{
		PropertyUpdateRequest updateRequest{};
		updateRequest.texture = texture;
		updateRequest.name = name;
		updateRequest.type = ShaderPropertyType::Texture;

		m_descriptorUpdateRequests.push_back(updateRequest);
	}

	void ShaderPropertiesBlock::SetProperty(std::string name, SPtr<GraphicsBuffer> buffer)
	{
		PropertyUpdateRequest updateRequest{};
		updateRequest.type = ShaderPropertyType::Buffer;
		updateRequest.name = name;
		updateRequest.buffer = buffer;

		m_descriptorUpdateRequests.push_back(updateRequest);
	}

	void ShaderPropertiesBlock::SetProperty(std::string name, std::vector<SPtr<Texture>>* textureArray)
	{
		PropertyUpdateRequest updateRequest{};
		updateRequest.type = ShaderPropertyType::TextureArray;
		updateRequest.name = name;
		updateRequest.textureArray = textureArray;

		m_descriptorUpdateRequests.push_back(updateRequest);
	}

	void ShaderPropertiesBlock::ApplyProperties(MaterialApplyPropertyGPUTarget target)
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
			}case ShaderPropertyType::TextureArray:
				ApplyTextureArray(request, target);
				break;
			default:
				break;
			}
		}

		//m_descriptorUpdateRequests.clear();
	}
	void ShaderPropertiesBlock::ApplyTexture(PropertyUpdateRequest& request, MaterialApplyPropertyGPUTarget target)
	{
		auto binding = target.shader->GetBindingByName(request.name.c_str());

		if (binding == nullptr)
		{
			//LOG("Trying to set non-existing texture {} for shader {}", request.name, shader->GetName());
			return;
		}

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = request.texture->vkImageViewHandle;
		imageInfo.sampler = request.texture->Sampler.vkHandle;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet imageWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		imageWrite.descriptorCount = 1;
		imageWrite.dstBinding = binding->index;
		imageWrite.dstSet = target.descriptorSets[binding->set];
		imageWrite.pImageInfo = &imageInfo;
		imageWrite.descriptorType = binding->type;

		//target.commands->PushDescriptorSets(GetBindingPoint(target.pipeline), target.pipeline->vkPipelineLayoutHandle, binding->set, 1, &imageWrite);

		target.graphics->UpdateDescriptorSet(1, &imageWrite);
	}
	void ShaderPropertiesBlock::ApplyBuffer(PropertyUpdateRequest& request, MaterialApplyPropertyGPUTarget target)
	{
		auto binding = target.shader->GetBindingByName(request.name.c_str());

		if (binding == nullptr)
		{
			LOG("Trying to set non-existing parameter {} for shader {}", request.name, target.shader->GetName());
			return;
		}

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = request.buffer->vkHandle;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet bufferWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		bufferWrite.descriptorCount = 1;
		bufferWrite.dstBinding = binding->index;
		bufferWrite.dstSet = target.descriptorSets[binding->set];
		bufferWrite.pBufferInfo = &bufferInfo;
		bufferWrite.descriptorType = binding->type;

		target.graphics->UpdateDescriptorSet(1, &bufferWrite);

	//	target.commands->PushDescriptorSets(GetBindingPoint(target.pipeline), target.pipeline->vkPipelineLayoutHandle, binding->set, 1, &bufferWrite);
	}
	void ShaderPropertiesBlock::ApplyTextureArray(PropertyUpdateRequest& request, MaterialApplyPropertyGPUTarget settings)
	{
		auto binding = settings.shader->GetBindingByName(request.name.c_str());

		if (binding == nullptr)
		{
			LOG("Trying to set non-existing parameter {} for shader {}", request.name, settings.shader->GetName());
			return;
		}

		static VkWriteDescriptorSet descriptorWrites[PuduGraphics::k_MAX_BINDLESS_RESOURCES];
		static VkDescriptorImageInfo imageInfos[PuduGraphics::k_MAX_BINDLESS_RESOURCES];

		auto textureArray = request.textureArray;
		uint32_t currentWriteIndex = 0;
		uint32_t setIndex;

		for (int i = 0; i < textureArray->size(); i++)
		{
			auto texture = textureArray->at(i);
			VkWriteDescriptorSet& descriptorWrite = descriptorWrites[currentWriteIndex];
			descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.dstArrayElement = texture->Handle();
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.dstSet = settings.descriptorSets[binding->set];

			descriptorWrite.dstBinding = binding->index;

			auto textureSampler = texture->Sampler;

			VkDescriptorImageInfo& descriptorImageInfo = imageInfos[currentWriteIndex];
			descriptorImageInfo.sampler = textureSampler.vkHandle;
			descriptorImageInfo.imageView = texture->vkImageViewHandle;
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			descriptorWrite.pImageInfo = &descriptorImageInfo;

			currentWriteIndex++;
			setIndex = binding->set;
		}

		settings.graphics->UpdateDescriptorSet(currentWriteIndex, descriptorWrites);
		//settings.commands->PushDescriptorSets(GetBindingPoint(settings.pipeline), settings.pipeline->vkPipelineLayoutHandle, setIndex, currentWriteIndex, descriptorWrites);
	}

	Material::Material(PuduGraphics* graphics)
	{
		this->Create(graphics);
	}

	void Material::SetShader(SPtr<Shader> shader)
	{
		m_shader = shader;

		m_gpu->CreateDescriptorSets(m_descriptorSets,shader->GetDescriptorSetLayoutsData()->setsCount,shader->GetDescriptorSetLayouts());
	}
}

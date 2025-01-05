#include "Material.h"
#include "PuduGraphics.h"
#include "PuduConstants.h"

namespace Pudu {
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

	void ShaderPropertiesBlock::ApplyProperties(PuduGraphics* graphics, IShaderObject* shader, Pipeline* pipeline)
	{
		for (auto& request : m_descriptorUpdateRequests)
		{
			switch (request.type)
			{
			case ShaderPropertyType::Texture:
			{
				ApplyTexture(request,graphics,shader,pipeline);
				break;
			}
			case ShaderPropertyType::Buffer:
			{
				ApplyBuffer(request, graphics, shader, pipeline);
				break;
			}case ShaderPropertyType::TextureArray:
				ApplyTextureArray(request, graphics, shader, pipeline);
				break;
			default:
				break;
			}
		}

		m_descriptorUpdateRequests.clear();
	}
	void ShaderPropertiesBlock::ApplyTexture(PropertyUpdateRequest& request, PuduGraphics* graphics, IShaderObject* shader, Pipeline* pipeline)
	{
		auto binding = shader->GetBindingByName(request.name.c_str());

		if (binding == nullptr)
		{
			LOG("Trying to set non-existing parameter {} for shader {}", request.name, shader->GetName());
			return;
		}

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = request.texture->vkImageViewHandle;
		imageInfo.sampler = request.texture->Sampler.vkHandle;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet imageWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		imageWrite.descriptorCount = 1;
		imageWrite.dstBinding = binding->index;
		imageWrite.dstSet = pipeline->vkDescriptorSets[binding->set];
		imageWrite.pImageInfo = &imageInfo;
		imageWrite.descriptorType = binding->type;
		graphics->UpdateDescriptorSet(1, &imageWrite);
	}
	void ShaderPropertiesBlock::ApplyBuffer(PropertyUpdateRequest& request, PuduGraphics* graphics, IShaderObject* shader, Pipeline* pipeline)
	{
		auto binding = shader->GetBindingByName(request.name.c_str());

		if (binding == nullptr)
		{
			LOG("Trying to set non-existing parameter {} for shader {}", request.name, shader->GetName());
			return;
		}

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = request.buffer->vkHandle;
		bufferInfo.offset = request.buffer->GetOffset();
		bufferInfo.range = request.buffer->GetSize();

		VkWriteDescriptorSet bufferWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		bufferWrite.descriptorCount = 1;
		bufferWrite.dstBinding = binding->index;
		bufferWrite.dstSet = pipeline->vkDescriptorSets[binding->set];
		bufferWrite.pBufferInfo = &bufferInfo;
		bufferWrite.descriptorType = binding->type;
		graphics->UpdateDescriptorSet(1, &bufferWrite);
	}
	void ShaderPropertiesBlock::ApplyTextureArray(PropertyUpdateRequest& request, PuduGraphics* graphics, IShaderObject* shader, Pipeline* pipeline)
	{
		auto binding = shader->GetBindingByName(request.name.c_str());

		if (binding == nullptr)
		{
			LOG("Trying to set non-existing parameter {} for shader {}", request.name, shader->GetName());
			return;
		}
		
		static VkWriteDescriptorSet bindlessDescriptorWrites[PuduGraphics::k_MAX_BINDLESS_RESOURCES];
		static VkDescriptorImageInfo bindlessImageInfos[PuduGraphics::k_MAX_BINDLESS_RESOURCES];

		auto textureArray = request.textureArray;
		uint32_t currentWriteIndex = 0;

		for (int i = 0; i < textureArray->size(); i++)
		{
			auto texture = textureArray->at(i);
			VkWriteDescriptorSet& descriptorWrite = bindlessDescriptorWrites[currentWriteIndex];
			descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.dstArrayElement = texture->Handle();
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.dstSet = pipeline->vkDescriptorSets[binding->set];

			descriptorWrite.dstBinding = binding->index;

			auto textureSampler = texture->Sampler;

			VkDescriptorImageInfo& descriptorImageInfo = bindlessImageInfos[currentWriteIndex];
			descriptorImageInfo.sampler = textureSampler.vkHandle;
			descriptorImageInfo.imageView = texture->vkImageViewHandle;
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			descriptorWrite.pImageInfo = &descriptorImageInfo;

			currentWriteIndex++;
		}

		if (currentWriteIndex)
		{
			graphics->UpdateDescriptorSet(currentWriteIndex, bindlessDescriptorWrites, 0, nullptr);
		}
	}
}

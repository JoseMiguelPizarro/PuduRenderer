#include "Material.h"
#include "PuduGraphics.h"
namespace Pudu {
	void Material::SetProperty(std::string name, SPtr<Pudu::Texture> texture)
	{
		//	m_propertiesBlock.SetProperty(name, texture);
	}

	void ShaderPropertiesBlock::SetProperty(std::string name, SPtr<Pudu::Texture> texture)
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

	void ShaderPropertiesBlock::ApplyProperties(PuduGraphics* graphics, IShaderObject* shader, Pipeline* pipeline)
	{
		for (auto& request : m_descriptorUpdateRequests)
		{
			auto binding = shader->GetBindingByName(request.name.c_str());


			switch (request.type)
			{
			case ShaderPropertyType::Texture:
			{
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
				break;
			}
			case ShaderPropertyType::Buffer:
			{
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
				break;
			}
			default:
				break;
			}
		}

		m_descriptorUpdateRequests.clear();
	}
}

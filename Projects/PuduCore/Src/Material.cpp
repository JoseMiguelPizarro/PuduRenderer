#include "Material.h"
#include "PuduGraphics.h"

namespace Pudu {
	void Material::SetProperty(std::string name, SPtr<Pudu::Texture> texture)
	{
		m_propertiesBlock.SetProperty(name, texture);
	}

	void ShaderPropertiesBlock::SetProperty(std::string name, SPtr<Pudu::Texture> texture)
	{
		PropertyUpdateRequest updateRequest{};
		updateRequest.texture = texture;
		updateRequest.name = name.c_str();

		m_descriptorUpdateRequests.push_back(updateRequest);
	}
	void ShaderPropertiesBlock::ApplyProperties(PuduGraphics* graphics, IShaderObject* shader, Pipeline* pipeline)
	{
		for (auto& request : m_descriptorUpdateRequests)
		{
			//For this to work each material will need its own descriptorset that will have to be bind each frame
			auto binding = shader->GetBindingByName(request.name);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageView = request.texture->vkImageViewHandle;
			imageInfo.sampler = request.texture->Sampler.vkHandle;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkWriteDescriptorSet imageWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			imageWrite.descriptorCount = 1;
			imageWrite.dstBinding = request.binding->index;
			imageWrite.dstSet = pipeline->vkDescriptorSets[request.binding->set];
			imageWrite.pImageInfo = &imageInfo;
			imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

			graphics->UpdateDescriptorSet(1, &imageWrite);
		}
		m_descriptorUpdateRequests.clear();
	}
}

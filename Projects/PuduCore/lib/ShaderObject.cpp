#include "ShaderObject.h"
#include "PuduGraphics.h"

namespace Pudu {

	void ShaderObject::Write(ShaderOffset offset, Texture* texture)
	{
		uint32_t bindingIndex = m_typeLayout->getFieldBindingRangeOffset(offset.bindingRangeIndex);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = texture->vkImageViewHandle;
		imageInfo.sampler = texture->Sampler.vkHandle;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet imageWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		imageWrite.dstSet = m_descriptorSet;
		imageWrite.descriptorCount = 1;
		imageWrite.dstBinding = bindingIndex;
		imageWrite.pImageInfo = &imageInfo;
		imageWrite.descriptorType = MapToDescriptorType(m_typeLayout->getBindingRangeType(bindingIndex));

		m_graphics->UpdateDescriptorSet(1, &imageWrite);
	}
	void ShaderObject::Write(ShaderOffset offset, const void* data, size_t size)
	{
		memcpy(m_bufferData + offset.byteOffset, data, size);
	}
}

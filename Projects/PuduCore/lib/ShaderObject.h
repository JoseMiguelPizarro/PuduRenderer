#pragma once
#include <vulkan/vulkan_core.h>
#include <slang.h>
#include <exception>
#include <memory>


namespace Pudu
{
	class PuduGraphics;
	class Texture;
	class TextureSampler;

	struct ShaderOffset {
		size_t			byteOffset = 0;
		uint32_t		bindingRangeIndex = 0;
		uint32_t 		arrayIndexInBindingRange = 0;
	};

	struct ShaderObject
	{
	public:
		ShaderObject(PuduGraphics* graphics):m_graphics(graphics)
		{

		}

		virtual void Write(ShaderOffset offset, Texture* texture);
		virtual void Write(ShaderOffset offset, TextureSampler* sampler);
		virtual void Write(ShaderOffset offset, const void* data, size_t size);


		static VkDescriptorType MapToDescriptorType(slang::BindingType bindingRangeType)
		{
			switch (bindingRangeType)
			{

			case slang::BindingType::Texture:
				return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			case slang::BindingType::MutableTexture:
				return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case slang::BindingType::CombinedTextureSampler:
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			default:
				throw std::exception("Binding range not supported");
			}
		}

	private:
		slang::TypeLayoutReflection* m_typeLayout;
		PuduGraphics* m_graphics;
		VkDescriptorSet m_descriptorSet;

		VkBuffer m_buffer;
		uint8_t* m_bufferData;
	};

}

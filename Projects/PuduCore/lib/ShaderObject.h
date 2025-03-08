#pragma once

#include "Texture.h"
#include "Resources/GPUResource.h"
#include "DescriptorSetLayoutInfo.h"

namespace Pudu {

	class Pipeline;
	class RenderPass;

	class IShaderObject
	{
	public:
		virtual DescriptorSetLayoutsCollection* GetDescriptorSetLayoutsData() { return &m_descriptorLayoutsData; };
		virtual SPtr<Pipeline> CreatePipeline(PuduGraphics* graphics, RenderPass* renderPass) = 0;
		virtual VkShaderModule GetModule() { return m_module; }
		DescriptorBinding* GetBindingByName(const char* name);
		virtual void SetName(const char* name) = 0;
		virtual const char* GetName() = 0;

		VkDescriptorSetLayout* GetVkDescriptorSetLayouts(){return m_VkDescriptorSetLayouts.data();}
		uint32_t GetActiveLayoutCount(){return numActiveLayouts;}


	protected:
		friend class PuduGraphics;
		void SetDescriptorSetLayouts(std::vector<SPtr<DescriptorSetLayout>> layouts)
		{
			descriptorSetLayouts = layouts;

			for (const auto& layout: descriptorSetLayouts)
			{
				m_VkDescriptorSetLayouts.push_back(layout->vkHandle);
			}

			numActiveLayouts = descriptorSetLayouts.size();
		};

		VkShaderModule m_module;
		DescriptorSetLayoutsCollection m_descriptorLayoutsData;
		std::vector<SPtr<DescriptorSetLayout>> descriptorSetLayouts;
		std::vector<GPUResourceHandle<DescriptorSetLayout>> m_descriptorSetLayoutHandles;
		std::vector<VkDescriptorSetLayout> m_VkDescriptorSetLayouts;

		uint32_t numActiveLayouts;
	};
}
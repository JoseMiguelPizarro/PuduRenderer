#pragma once

#include "Texture.h"
#include "Resources/GPUResource.h"
#include "DescriptorSetLayoutData.h"

namespace Pudu {

	class Pipeline;
	class RenderPass;

	class IShaderObject
	{
	public:
		virtual DescriptorSetLayoutsData* GetDescriptorSetLayoutsData() { return &m_descriptorLayoutsData; };
		virtual SPtr<Pipeline> CreatePipeline(PuduGraphics* graphics, RenderPass* renderPass) = 0;
		VkShaderModule GetModule() { return m_module; }
		DescriptorBinding* GetBindingByName(const char* name);
		virtual void SetName(const char* name) = 0;
		virtual const char* GetName() = 0;

		std::vector<VkDescriptorSetLayout>* GetVkDescriptorSetLayouts(){return &m_VkpipelineDescriptorSetLayouts;}


	protected:
		friend class PuduGraphics;
		void SetDescriptorSetLayouts(std::vector<SPtr<DescriptorSetLayout>> layouts)
		{
			descriptorSetLayouts = layouts;

			for (const auto& layout: descriptorSetLayouts)
			{
				m_VkpipelineDescriptorSetLayouts.push_back(layout->vkHandle);
			}
		};

		VkShaderModule m_module;
		DescriptorSetLayoutsData m_descriptorLayoutsData;
		std::vector<SPtr<DescriptorSetLayout>> descriptorSetLayouts;
		std::vector<GPUResourceHandle<DescriptorSetLayout>> m_descriptorSetLayoutHandles;
		std::vector<VkDescriptorSetLayout> m_VkpipelineDescriptorSetLayouts;


		uint32_t numActiveLayouts;
	};
}
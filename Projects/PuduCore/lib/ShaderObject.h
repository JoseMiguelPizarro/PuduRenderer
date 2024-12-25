#pragma once

#include <string>
#include "Texture.h"
#include "Resources/GPUResource.h"

namespace Pudu {

	class Pipeline;
	class RenderPass;

	class IShaderObject
	{
	public:
		virtual DescriptorSetLayoutsData GetDescriptorSetLayouts() { return m_descriptors; };
		virtual SPtr<Pipeline> CreatePipeline(PuduGraphics* graphics, RenderPass* renderPass) = 0;
		VkShaderModule GetModule() { return m_module; }
		DescriptorBinding* GetBindingByName(const char* name);
		const char* Name() { return m_name.c_str(); }

	protected:
		friend class PuduGraphics;
		VkShaderModule m_module;
		DescriptorSetLayoutsData m_descriptors;
		std::string m_name;
	};
}
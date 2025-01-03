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
		virtual void SetName(const char* name) = 0;
		virtual const char* GetName() = 0;

	protected:
		friend class PuduGraphics;
		VkShaderModule m_module;
		DescriptorSetLayoutsData m_descriptors;
	};
}
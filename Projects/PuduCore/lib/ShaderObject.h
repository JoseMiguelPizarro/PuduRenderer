#pragma once

#include <string>
#include "Texture.h"

namespace Pudu {
	class IShaderObject
	{
	public:
		virtual DescriptorSetLayoutsData GetDescriptorSetLayouts() { return m_descriptors; };
		VkShaderModule GetModule(){ return m_module; }
		DescriptorBinding* GetBindingByName(const char* name);


	protected:
		friend class PuduGraphics;
		VkShaderModule m_module;
		DescriptorSetLayoutsData m_descriptors;
	};
}
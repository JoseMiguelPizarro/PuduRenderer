#include "ShaderObject.h"

namespace Pudu
{
	DescriptorBinding* IShaderObject::GetBindingByName(const char* name)
	{
		auto bindingData = &m_descriptorLayoutsData.bindingsData;
		for (size_t i = 0; i < bindingData->size(); i++)
		{
			DescriptorBinding* binding = &(bindingData->at(i));
			if (binding->name == name)
			{
				return binding;
			}
		}

		return nullptr;
	}
}

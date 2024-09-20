#include "Material.h"

namespace Pudu {
	void Material::SetProperty(std::string name, SPtr<Pudu::Texture> texture)
	{
		DescriptorBinding* shaderBinding = nullptr;

		std::vector<DescriptorBinding>* bindingData = &this->Shader->descriptors.bindingsData;
		for (size_t i = 0; i < bindingData->size(); i++)
		{
			DescriptorBinding* binding = &(bindingData->at(i));
			if (binding->name == name)
			{
				shaderBinding = binding;
			}
		}

		DescriptorUpdateRequest updateRequest{};
		updateRequest.binding = shaderBinding;
		updateRequest.texture = texture;

		descriptorUpdateRequests.push_back(updateRequest);
	}
}

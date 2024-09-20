#include <spirv_reflect.h>
#include <vulkan/vulkan.h>
#include "SPIRVParser.h"
#include <iostream>
#include <boolinq.h>

using namespace boolinq;

namespace Pudu {
	void SPIRVParser::GetDescriptorSetLayout(const char* spirvData, uint32_t size, DescriptorsCreationData& outDescriptorSetLayoutData)
	{
		SpvReflectShaderModule module{};
		SpvReflectResult result = spvReflectCreateShaderModule2(SPV_REFLECT_MODULE_FLAG_NONE, size, spirvData, &module);

		uint32_t count = 0;
		result = spvReflectEnumerateDescriptorSets(&module, &count, nullptr);

		std::vector<SpvReflectDescriptorSet*> sets(count);
		result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());

		for (size_t setIndex = 0; setIndex < sets.size(); ++setIndex)
		{
			std::vector<DescriptorSetLayoutData*> layoutsPtr;

			for (size_t i = 0; i < outDescriptorSetLayoutData.layoutData.size(); i++)
			{
				layoutsPtr.push_back(&outDescriptorSetLayoutData.layoutData[i]);
			}

			const SpvReflectDescriptorSet& reflSet = *(sets[setIndex]);
			DescriptorSetLayoutData layout{};

			//Create a proxy list that holds pointers to the sets in outDescriptorSetLayoutData, so we can return the pointer in the query
			auto storedLayout = from(layoutsPtr)
				.where([reflSet](const DescriptorSetLayoutData* l) {return l->SetNumber == reflSet.set; })
				.firstOrDefault(&layout);

			bool layoutExists = from(layoutsPtr).any([reflSet](const DescriptorSetLayoutData* l) { return l->SetNumber == reflSet.set; });


			for (uint32_t bindingIndex = 0; bindingIndex < reflSet.binding_count; ++bindingIndex) {

				const SpvReflectDescriptorBinding& refl_binding = *(reflSet.bindings[bindingIndex]);

				auto bindingExists = from(storedLayout->Bindings)
					.any([refl_binding](const VkDescriptorSetLayoutBinding b) {return b.binding == refl_binding.binding; });

				if (bindingExists)
				{
					std::vector<VkDescriptorSetLayoutBinding*> bindingPtrs;

					for (size_t i = 0; i < storedLayout->Bindings.size(); i++)
					{
						bindingPtrs.push_back(&storedLayout->Bindings[i]);
					}

					auto bindingPtr = from(bindingPtrs)
						.first([refl_binding](const VkDescriptorSetLayoutBinding* b)
					{ return b->binding == refl_binding.binding; });

					bindingPtr->stageFlags |= static_cast<VkShaderStageFlagBits>(module.shader_stage);
				}
				else
				{
					VkDescriptorSetLayoutBinding layoutBinding = {};
					layoutBinding.binding = refl_binding.binding;
					layoutBinding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);
					layoutBinding.descriptorCount = 1;
					for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
						layoutBinding.descriptorCount *= refl_binding.array.dims[i_dim];
					}
					layoutBinding.stageFlags = static_cast<VkShaderStageFlagBits>(module.shader_stage);
					layoutBinding.pImmutableSamplers = nullptr;

					storedLayout->Bindings.push_back(layoutBinding);

					DescriptorBinding binding;
					binding.name = refl_binding.name;
					binding.set = refl_binding.set;
					binding.index = refl_binding.binding;

					outDescriptorSetLayoutData.bindingsData.push_back(binding);
				}
			}

			if (!layoutExists)
			{
				layout.SetNumber = reflSet.set;
				layout.CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layout.CreateInfo.bindingCount = reflSet.binding_count; //We might have issues here if the binding count doesn't consider the ones in vertex/fragment
				layout.CreateInfo.pBindings = layout.Bindings.data();
				outDescriptorSetLayoutData.layoutData.push_back(layout);
			}
		}

		outDescriptorSetLayoutData.setsCount = outDescriptorSetLayoutData.layoutData.size();
		spvReflectDestroyShaderModule(&module);
	}

	bool SortBySetNumber(DescriptorSetLayoutData a, DescriptorSetLayoutData b)
	{
		return a.SetNumber < b.SetNumber;
	}

	void SPIRVParser::GetDescriptorSetLayout(Shader* creationData, DescriptorsCreationData& outDescriptorSetLayoutData)
	{
		if (creationData->vertexData.size() > 0)
		{
			GetDescriptorSetLayout(creationData->vertexData.data(), creationData->vertexData.size() * sizeof(char), outDescriptorSetLayoutData);
		}

		if (creationData->fragmentData.size() > 0)
		{
			GetDescriptorSetLayout(creationData->fragmentData.data(), creationData->fragmentData.size() * sizeof(char), outDescriptorSetLayoutData);
		}

		std::sort(outDescriptorSetLayoutData.layoutData.begin(), outDescriptorSetLayoutData.layoutData.end(), SortBySetNumber);
	}
}
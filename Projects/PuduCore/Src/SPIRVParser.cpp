#include <spirv_reflect.h>
#include <vulkan/vulkan.h>
#include "SPIRVParser.h"
#include <iostream>

namespace Pudu {
	void SPIRVParser::GetDescriptorSetLayout(const char* spirvData, uint32_t size, std::vector<DescriptorSetLayoutData>& outDescriptorSetLayoutData)
	{
		SpvReflectShaderModule module{};
		SpvReflectResult result = spvReflectCreateShaderModule2(SPV_REFLECT_MODULE_FLAG_NONE, size, spirvData, &module);

		uint32_t count = 0;
		result = spvReflectEnumerateDescriptorSets(&module, &count, nullptr);

		std::vector<SpvReflectDescriptorSet*> sets(count);
		result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());

		for (size_t setIndex = 0; setIndex < sets.size(); ++setIndex)
		{
			const SpvReflectDescriptorSet& reflSet = *(sets[setIndex]);
			DescriptorSetLayoutData layout{};

			layout.Bindings.resize(reflSet.binding_count);
			for (uint32_t bindingIndex = 0; bindingIndex < reflSet.binding_count; ++bindingIndex) {
				const SpvReflectDescriptorBinding& refl_binding = *(reflSet.bindings[bindingIndex]);
				VkDescriptorSetLayoutBinding& layoutBinding = layout.Bindings[bindingIndex];
				layoutBinding.binding = refl_binding.binding;
				layoutBinding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);
				layoutBinding.descriptorCount = 1;
				for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
					layoutBinding.descriptorCount *= refl_binding.array.dims[i_dim];
				}
				layoutBinding.stageFlags = static_cast<VkShaderStageFlagBits>(module.shader_stage);
			}

			layout.SetNumber = reflSet.set;
			layout.CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layout.CreateInfo.bindingCount = reflSet.binding_count;
			layout.CreateInfo.pBindings = layout.Bindings.data();

			outDescriptorSetLayoutData.push_back(layout);
		}

		spvReflectDestroyShaderModule(&module);
	}
	void SPIRVParser::GetDescriptorSetLayout(PipelineCreationData& creationData, std::vector<DescriptorSetLayoutData>& outDescriptorSetLayoutData)
	{
		GetDescriptorSetLayout(creationData.vertexShaderData.data(), creationData.vertexShaderData.size() * sizeof(char), outDescriptorSetLayoutData);
		GetDescriptorSetLayout(creationData.fragmentShaderData.data(), creationData.fragmentShaderData.size() * sizeof(char), outDescriptorSetLayoutData);
	}
}
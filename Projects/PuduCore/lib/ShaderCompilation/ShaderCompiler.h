#pragma once
#include <slang.h>
#include <slang-com-ptr.h>
#include <vector>
#include <unordered_map>
#include "DescriptorSetLayoutData.h"
using namespace slang;


namespace Pudu {

	struct ShaderKernel {
		const u32* code;
		size codeSize;
	};

	struct ShaderCompilation {
	public:
		DescriptorSetLayoutsData descriptorsData;
		ShaderKernel* GetKernel(const char* name) { return &m_kernelsByName[name]; }
		void AddKernel(const char* name, ShaderKernel& kernel);
		std::vector<VkPushConstantRange>* GetPushConstantRanges() { return &m_pushConstantRanges;}

	private:
		friend class DescriptorsBuilder;
		std::unordered_map<std::string, ShaderKernel> m_kernelsByName;
		std::vector<VkPushConstantRange> m_pushConstantRanges;
	};

	struct ShaderObjectLayoutBuilder
	{
		std::vector<VkDescriptorSetLayoutBinding> m_bindings;
		u32 m_bindingIndex = 0;
		void addBindingsForParameterBlock(
			slang::TypeLayoutReflection* typeLayout, DescriptorSetLayoutsData& layoutsData);
		void addBindingsFrom(
			slang::TypeLayoutReflection* typeLayout,
			u32 descriptorCount);

		Slang::ComPtr<IGlobalSession> m_globalSession;
	};


	class ShaderCompiler {
	public:
		void Init();
		ShaderCompilation Compile(const char* path, std::vector<const char*> entryPoints, bool compute = false);

	private:
		Slang::ComPtr<ISession> m_session;
		Slang::ComPtr<IGlobalSession> m_globalSession;
	};
}
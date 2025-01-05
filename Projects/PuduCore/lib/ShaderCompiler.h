#pragma once
#include <slang.h>
#include <slang-com-ptr.h>
#include <vector>
#include <unordered_map>
#include "DescriptorSetLayoutData.h"
using namespace slang;


namespace Pudu {

	struct ShaderKernel {
		const uint32_t* code;
		size_t codeSize;
	};

	struct ShaderCompilation {
	public:
		DescriptorSetLayoutsData descriptorsData;
		ShaderKernel* GetKernel(const char* name) { return &m_kernelsByName[name]; }
		void AddKernel(const char* name, ShaderKernel& kernel);

	private:
		std::unordered_map<std::string, ShaderKernel> m_kernelsByName;
	};


	struct ShaderObjectLayoutBuilder
	{
		std::vector<VkDescriptorSetLayoutBinding> m_bindings;
		uint32_t m_bindingIndex = 0;
		void addBindingsForParameterBlock(
			slang::TypeLayoutReflection* typeLayout, DescriptorSetLayoutsData& layoutsData);
		void addBindingsFrom(
			slang::TypeLayoutReflection* typeLayout,
			uint32_t descriptorCount);

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

#include <vulkan/vulkan_core.h>
#include "ShaderCompilation/ShaderCompiler.h"
#include "FileManager.h"
#include <iostream>
#include <boolinq.h>

#include "Logger.h"

using namespace boolinq;


void PrintVariable(VariableReflection* variable) {
	printf("%s", variable->getName());

}

void PrintType(TypeReflection* type) {
	printf(type->getName());

}

namespace Pudu {





	void ShaderCompiler::Init()
	{
		createGlobalSession(m_globalSession.writeRef());

		TargetDesc targetDesc;
		targetDesc.profile = m_globalSession->findProfile("spirv_1_6");
		targetDesc.format = SLANG_SPIRV;

		const char* searchPaths[] = { "Assets/Shaders" };

		SessionDesc sessionDesc;
		sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
		sessionDesc.targetCount = 1;
		sessionDesc.targets = &targetDesc;
		sessionDesc.searchPathCount = 1;
		sessionDesc.searchPaths = searchPaths;

		slang::CompilerOptionEntry useEntryPointNameOption;
		useEntryPointNameOption.name = slang::CompilerOptionName::VulkanUseEntryPointName;
		useEntryPointNameOption.value = { slang::CompilerOptionValueKind::Int,1, 0, nullptr, nullptr };

		std::array<slang::CompilerOptionEntry, 1> options =
		{
			useEntryPointNameOption
		};

		sessionDesc.compilerOptionEntries = options.data();
		sessionDesc.compilerOptionEntryCount = options.size();


		m_globalSession->createSession(sessionDesc, m_session.writeRef());
	}

	void PrintDiagnostics(Slang::ComPtr<IBlob> diagnostics)
	{
		if (diagnostics)
		{
			fprintf(stderr, "%s\n", static_cast<const char*>(diagnostics->getBufferPointer()));
		}
	}

	ShaderCompilation ShaderCompiler::Compile(const char* path, std::vector<const char*> entryPoints, bool compute)
	{
		Slang::ComPtr<IBlob> diagnostics;
		IModule* baseModule = m_session->loadModule("PuduGraphics.slang");
		IModule* module = m_session->loadModule(path, diagnostics.writeRef());

		PrintDiagnostics(diagnostics);

		std::vector<Slang::ComPtr<IEntryPoint>> slangEntryPoints;

		for (auto entryPoint : entryPoints) {
			Slang::ComPtr<IEntryPoint> e;

			module->findEntryPointByName(entryPoint, e.writeRef());

			slangEntryPoints.push_back(e);
		}

		std::vector<IComponentType*> components = {};
		if (!compute)
		{
			components.push_back(baseModule);
		}

		components.push_back(module);

		components.append_range(slangEntryPoints);

		Slang::ComPtr<IComponentType> program;
		m_session->createCompositeComponentType(components.data(), components.size(), program.writeRef(), diagnostics.writeRef());

		PrintDiagnostics(diagnostics);

		slang::ProgramLayout* layout = program->getLayout();



		Slang::ComPtr<IComponentType> linkedProgram;
		program->link(linkedProgram.writeRef(), diagnostics.writeRef());

		PrintDiagnostics(diagnostics);

		//Global
		printf("Global scope \n");
		ShaderCompilation compiledData;

		ShaderObjectLayoutBuilder layoutBuilder;
		layoutBuilder.m_globalSession = m_globalSession;
		layoutBuilder.ParseShaderProgramLayout(layout, compiledData.descriptorsData);



		PrintDiagnostics(diagnostics);

		for (size_t i = 0; i < entryPoints.size(); i++)
		{
			Slang::ComPtr<IBlob> kernel;
			linkedProgram->getEntryPointCode(i, 0, kernel.writeRef(), diagnostics.writeRef());

			ShaderKernel kernelData;
			kernelData.codeSize = kernel->getBufferSize();

			const auto codePtr = malloc(kernelData.codeSize);
			memcpy(codePtr, kernel->getBufferPointer(), kernelData.codeSize);

			kernelData.code = static_cast<const uint32_t*>(codePtr);
			compiledData.AddKernel(entryPoints[i], kernelData);
			PrintDiagnostics(diagnostics);
		}

		return compiledData;
	}

	VkDescriptorType ToVk(slang::BindingType slangBindingType)
	{
		switch (slangBindingType)
		{
		case slang::BindingType::PushConstant:
		default:
			std::fprintf(stderr, "Assertion failed: %s %d\n", "Unsupported binding type!", slangBindingType);
			//std::abort();
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;

		case slang::BindingType::Sampler:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case slang::BindingType::CombinedTextureSampler:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case slang::BindingType::Texture:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case slang::BindingType::MutableTexture:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case slang::BindingType::TypedBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case slang::BindingType::MutableTypedBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case slang::BindingType::RawBuffer:
		case slang::BindingType::MutableRawBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case slang::BindingType::InputRenderTarget:
			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		case slang::BindingType::InlineUniformData:
			return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
		case slang::BindingType::RayTracingAccelerationStructure:
			return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		case slang::BindingType::ConstantBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
	}

	void Tab(uint32_t count) {
		for (size_t i = 0; i < count; i++)
		{
			printf("%s", "\t");
		}
	}



}




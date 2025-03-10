
#include <vulkan/vulkan_core.h>
#include "ShaderCompilation/ShaderObjectLayoutBuilder.h"
#include "ShaderCompilation/ShaderCompiler.h"
#include "FileManager.h"

#include "Logger.h"

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

	ShaderCompilationObject	ShaderCompiler::Compile(const char* path, const std::vector<const char*>& entryPoints, bool compute) const
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
		LOG("Global scope : {}", path);
		ShaderCompilationObject compiledData;
		ShaderObjectLayoutBuilder layoutBuilder;
		layoutBuilder.m_globalSession = m_globalSession;
		layoutBuilder.ParseShaderProgramLayout(layout, compiledData);

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

	ShaderCompilationObject ShaderCompiler::CompileModule(const fs::path& path)
	{
		ASSERT(!path.empty(),"Cannot compile module. Path is empty");
		Slang::ComPtr<IBlob> diagnostics;
		IModule* module = m_session->loadModule(path.string().c_str(), diagnostics.writeRef());

		PrintDiagnostics(diagnostics);

		std::vector<IComponentType*> components = {};

		components.push_back(module);

		Slang::ComPtr<IComponentType> program;
		m_session->createCompositeComponentType(components.data(), components.size(), program.writeRef(), diagnostics.writeRef());

		PrintDiagnostics(diagnostics);

		slang::ProgramLayout* layout = program->getLayout();

		Slang::ComPtr<IComponentType> linkedProgram;
		program->link(linkedProgram.writeRef(), diagnostics.writeRef());

		PrintDiagnostics(diagnostics);
		//Global
		LOG("Global scope : {}", path.string().c_str());
		ShaderCompilationObject compiledData;
		ShaderObjectLayoutBuilder layoutBuilder;
		layoutBuilder.m_globalSession = m_globalSession;
		layoutBuilder.ParseShaderProgramLayout(layout, compiledData);

		PrintDiagnostics(diagnostics);

		return compiledData;
	}
}


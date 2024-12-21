#include <slang.h>
#include "ShaderCompiler.h"
#include "Paths.h"

namespace Pudu {


	void Pudu::ShaderCompiler::Init()
	{
		slang::createGlobalSession(m_globalSession.writeRef());

		slang::SessionDesc sessionDesc;

		slang::TargetDesc targetDesc;
		targetDesc.format = SLANG_SPIRV;
		targetDesc.profile = m_globalSession->findProfile("glsl_450");
		sessionDesc.targets = &targetDesc;
		sessionDesc.targetCount = 1;

		auto path = ASSETS_FOLDER_PATH;
		path.append("Shaders");
		const char* searchPath[] = { path.string().c_str() };
		sessionDesc.searchPaths = searchPath;

		sessionDesc.preprocessorMacroCount = 0;
		sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

		m_globalSession->createSession(sessionDesc, m_session.writeRef());
	}

	ShaderCompiledObject Pudu::ShaderCompiler::LoadShader(fs::path path)
	{
		Slang::ComPtr<slang::IBlob> diagnostics;
		auto module = m_session->loadModule(path.string().c_str(), diagnostics.writeRef());

		if (diagnostics)
		{
			fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
		}

		//Find entry points
		Slang::ComPtr<slang::IEntryPoint> fragmentEntryPoint;
		Slang::ComPtr<slang::IEntryPoint> vertexEntryPoint;

		module->findEntryPointByName("MainFragment", fragmentEntryPoint.writeRef());
		module->findEntryPointByName("MainVertex", vertexEntryPoint.writeRef());

		//Composite shader
		slang::IComponentType* components[] = { module,fragmentEntryPoint };
		Slang::ComPtr<slang::IComponentType> program;
		m_session->createCompositeComponentType(components, 2, program.writeRef());
		Slang::ComPtr<slang::IBlob> kernelBlob;
		program->getEntryPointCode(0, 0, kernelBlob.writeRef(), diagnostics.writeRef());

		Slang::ComPtr<slang::IComponentType> linkedProgram;
		program->link(linkedProgram.writeRef(), diagnostics.writeRef());


		return { module,program };
	}
}
#pragma once
#include <slang.h>
#include <slang-com-ptr.h>
#include <vector>
#include "ShaderCompilationObject.h"
using namespace slang;


namespace Pudu {
	class ShaderCompiler {
	public:
		void Init();
		ShaderCompilationObject Compile(const char* path, const std::vector<const char*>& entryPoints, bool compute = false) const;
		ShaderCompilationObject CompileModule(const fs::path& path);

	private:
		Slang::ComPtr<ISession> m_session;
		Slang::ComPtr<IGlobalSession> m_globalSession;
	};
}
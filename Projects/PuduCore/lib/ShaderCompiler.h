#pragma once
#include <filesystem>
#include <slang.h>
#include <slang-gfx.h>


namespace fs = std::filesystem;

namespace Pudu
{
	struct ShaderCompiledObject {
		slang::IModule* module;
		Slang::ComPtr<slang::IComponentType> program;
	};


	class ShaderCompiler {
	public:
		void Init();
		ShaderCompiledObject LoadShader(fs::path path);
	private:
		Slang::ComPtr<slang::IGlobalSession> m_globalSession;
		Slang::ComPtr<slang::ISession> m_session;
	};
}

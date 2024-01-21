#pragma once
#include <filesystem>
#include "PuduCore.h"
#include "Shader.h"

namespace Pudu {
	class ShaderManager
	{
	public:
		static SPtr<Shader> AllocateShader(std::filesystem::path path);
	};
}


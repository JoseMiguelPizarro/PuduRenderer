#include "PuduGraphics.h"
#include "ShaderManager.h"
#include "FileManager.h"

namespace Pudu {
	SPtr<Shader> ShaderManager::AllocateShader(std::filesystem::path path)
	{
		auto vertShaderCode = FileManager::ReadAssetFile("Shaders/Compiled/triangle.vert.spv");
		auto fragShaderCode = FileManager::ReadAssetFile("Shaders/Compiled/triangle.frag.spv");
		return SPtr<Shader>(); //Return empty shader for now, I'll set this up once I figure out auto-pipeline creation
	}
}

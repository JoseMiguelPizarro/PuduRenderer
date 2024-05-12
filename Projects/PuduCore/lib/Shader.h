#pragma once
#include <string>
#include <filesystem>
#include <vulkan/vulkan_core.h>
#include "Resources/Resources.h"

namespace Pudu {
	class Shader
	{
	public:
		ShaderHandle handle;
		VkShaderModule GetModule();
		std::filesystem::path GetPath();

		Shader() = default;
		Shader(VkShaderModule module) : m_module(module) {}
		Shader(std::filesystem::path path, VkShaderModule module) :m_shaderPath(path), m_module(module) {}

		std::vector<char> fragmentData;
		std::vector<char> vertexData;
		std::string name;

	private:
		std::filesystem::path m_shaderPath;
		VkShaderModule m_module;
	};
}


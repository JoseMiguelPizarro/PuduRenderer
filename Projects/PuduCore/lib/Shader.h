#pragma once
#include <vulkan/vulkan_core.h>
#include <filesystem>

namespace Pudu {
	class Shader
	{
	public:
		VkShaderModule GetModule();
		std::filesystem::path GetPath();

		Shader(VkShaderModule module) : m_module(module) {}
		Shader(std::filesystem::path path, VkShaderModule module) :m_shaderPath(path), m_module(module) {}
		

	private:
		std::filesystem::path m_shaderPath;
		VkShaderModule m_module;
	};
}


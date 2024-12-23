#pragma once
#include <string>
#include <filesystem>
#include <vulkan/vulkan_core.h>
#include "Resources/GPUResource.h"
#include  "Resources/Resources.h"
#include "ShaderObject.h"

namespace Pudu
{
	class Shader : public GPUResource<Shader>, public IShaderObject
	{
	public:
		VkShaderModule GetModule();
		std::filesystem::path GetPath();

		Shader() = default;

		Shader(VkShaderModule module)
		{
			this->m_module = module;
		}

		Shader(std::filesystem::path path, VkShaderModule module) : m_shaderPath(path)
		{
			this->m_module = module;
		}


		void LoadFragmentData(std::vector<char> data);
		void LoadVertexData(std::vector<char> data);

		std::vector<char> fragmentData;
		std::vector<char> vertexData;
		std::string name;

		bool HasFragmentData() { return m_hasFragmentData; }
		bool HasVertexData() { return m_hasVertexData; }

	private:
		friend class PuduGraphics;
		std::filesystem::path m_shaderPath;
		bool m_hasFragmentData;
		bool m_hasVertexData;
	};
}

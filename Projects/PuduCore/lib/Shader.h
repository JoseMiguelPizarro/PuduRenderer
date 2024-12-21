#pragma once
#include <string>
#include <filesystem>
#include <vulkan/vulkan_core.h>
#include "Resources/GPUResource.h"
#include  "Resources/Resources.h"
#include "ShaderCompiler.h"

namespace Pudu
{
	class Shader : public GPUResource<Shader>
	{
	public:
		VkShaderModule GetModule();
		std::filesystem::path GetPath();

		Shader() = default;

		Shader(VkShaderModule module) : m_module(module)
		{
		}

		Shader(std::filesystem::path path, VkShaderModule module) : m_shaderPath(path), m_module(module)
		{
		}


		void LoadFragmentData(const void* data, size_t size);
		void LoadVertexData(const void* data, size_t size);
		ShaderCompiledObject* GetCompiledObject() { return &m_shaderCompiledObject; }

		const void* fragmentData;
		size_t fragmentDataSize;
		const void* vertexData;
		size_t vertexDataSize;
		std::string name;

		bool HasFragmentData() { return m_hasFragmentData; }
		bool HasVertexData() { return m_hasVertexData; }

		DescriptorsCreationData descriptors;

	private:
		friend class PuduGraphics;
		std::filesystem::path m_shaderPath;
		VkShaderModule m_module;
		bool m_hasFragmentData;
		bool m_hasVertexData;
		ShaderCompiledObject m_shaderCompiledObject;
	};
}

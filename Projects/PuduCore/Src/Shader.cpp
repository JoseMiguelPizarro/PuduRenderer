#include "Shader.h"

VkShaderModule Pudu::Shader::GetModule()
{
	return VkShaderModule();
}

std::filesystem::path Pudu::Shader::GetPath()
{
	return std::filesystem::path();
}

void Pudu::Shader::LoadFragmentData(const void* data, size_t size)
{
	m_hasFragmentData = size > 0;

	fragmentData = data;
	fragmentDataSize = size;
}

void Pudu::Shader::LoadVertexData(const void* data, size_t size)
{
	m_hasVertexData = size > 0;

	vertexData = data;
	vertexDataSize = size;
}

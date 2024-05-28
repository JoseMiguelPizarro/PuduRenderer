#include "Shader.h"

VkShaderModule Pudu::Shader::GetModule()
{
	return VkShaderModule();
}

std::filesystem::path Pudu::Shader::GetPath()
{
	return std::filesystem::path();
}

void Pudu::Shader::LoadFragmentData(std::vector<char> data)
{
	m_hasFragmentData = data.size() > 0;

	fragmentData.append_range(data);
}

void Pudu::Shader::LoadVertexData(std::vector<char> data)
{
	m_hasFragmentData = data.size() > 0;

	vertexData.append_range(data);
}

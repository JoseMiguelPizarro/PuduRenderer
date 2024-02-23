#include <fmt/format.h>
#include "Logger.h"
#include "Resources/Resources.h"

namespace Pudu
{
	VertexInputCreation& VertexInputCreation::AddVertexStream(const VertexStream& stream)
	{
		vertexStreams[numVertexStreams++] = stream;

		return *this;
	}
	VertexInputCreation& VertexInputCreation::AddVertexAttribute(const VertexAttribute& attribute)
	{
		vertexAttributes[numVertexAttributes++] = attribute;

		return *this;
	}
	ShaderStateCreationData& ShaderStateCreationData::SetName(const char* name)
	{
		this->name = name;
		return *this;
	}
	ShaderStateCreationData& ShaderStateCreationData::AddStage(const char* code, size_t code_size, VkShaderStageFlagBits type)
	{
		ShaderStage stage;
		stage.code = code;
		stage.codeSize = code_size;
		stage.type = type;

		stages[stageCount++] = stage;

		return *this;
	}

	ShaderStage& ShaderStateCreationData::GetStage(VkShaderStageFlagBits stageFlag)
	{
		for (uint32_t i; i < stageCount; i++)
		{
			auto& stage = stages[i];

			if (stage.type == stageFlag)
			{
				return stage;
			}
		}

		PUDU_ERROR(fmt::format("Trying to get non valid stage {}", (uint32_t)stageFlag).c_str());
	}
}

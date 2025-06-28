#include "ShaderObject.h"

#include "Pipeline.h"

namespace Pudu
{
	SPtr<Pipeline> IShaderObject::CreatePipeline(PuduGraphics* graphics, RenderPass* renderPass)
	{
		auto pipeline = OnCreatePipeline(graphics, renderPass);
		m_pipelines.push_back(pipeline->Handle());
		return pipeline;
	}

	ShaderNode* IShaderObject::GetShaderLayout()
	{
		return m_compilationObject.descriptorsData.GetShaderLayout();
	}
}

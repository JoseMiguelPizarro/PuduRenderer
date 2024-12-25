#include "ComputeShader.h"
#include "PuduGraphics.h"
#include "SPIRVParser.h"
namespace Pudu
{
	void ComputeShader::SetTexture(const char* name, SPtr<Texture> texture)
	{
		m_propertiesBlock.SetProperty(name, texture);
	}
	void ComputeShader::SetBuffer(const char* name, SPtr<GraphicsBuffer> buffer)
	{
		m_propertiesBlock.SetProperty(name, buffer);
	}
	SPtr<Pipeline> ComputeShader::CreatePipeline(PuduGraphics* graphics, RenderPass* renderPass)
	{
		if (m_pipelineHandle.IsValid())
		{
			return graphics->Resources()->GetPipeline(m_pipelineHandle);
		}

		ComputePipelineCreationData creationData{};
		creationData.computeShaderHandle = Handle();
		creationData.name = Name();
		creationData.kernel = "main";
		creationData.descriptorsCreationData = m_descriptors;

		m_pipelineHandle =  graphics->CreateComputePipeline(creationData);

		return graphics->Resources()->GetPipeline(m_pipelineHandle);
	}
}

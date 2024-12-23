#include "PuduGraphics.h"
#include "ComputeRenderPass.h"
#include "RenderFrameData.h"
#include "Texture.h"

namespace Pudu
{
	void ComputeRenderPass::SetGroupSize(uint32_t x, uint32_t y, uint32_t z)
	{
		m_groupX = x;
		m_groupY = y;
		m_groupZ = z;
	}
	void ComputeRenderPass::SetKernel(std::string& name)
	{
		m_kernel = name;
	}
	void ComputeRenderPass::SetShader(SPtr<ComputeShader> shader)
	{
		m_shader = shader;
	}

	void ComputeRenderPass::Render(RenderFrameData& frameData)
	{
		auto pipeline = frameData.graphics->Resources()->GetPipeline(m_shader->pipelineHandle);

		auto computeCommands = frameData.frame->ComputeCommandBuffer;

		computeCommands->BindPipeline(pipeline.get());


		for (size_t i = 0; i < attachments.ColorAttachmentCount(); i++)
		{
			auto attachment = attachments.colorAttachments[i];

			m_shader->SetTexture(attachment.resource->name.c_str(), attachment.resource);
		}

		m_shader->GetPropertiesBlock()->ApplyProperties(frameData.graphics, m_shader.get(), pipeline.get());


		computeCommands->BindDescriptorSetCompute(pipeline->vkPipelineLayoutHandle, pipeline->vkDescriptorSets, pipeline->descriptorSetLayoutHandles.size());

		computeCommands->Dispatch(m_groupX, m_groupY, m_groupZ);
	}
}
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
		auto pipeline = frameData.graphics->Resources()->GetPipeline(m_shader->GetPipelineHandle());

		auto computeCommands = frameData.frame->ComputeCommandBuffer;

		computeCommands->BindPipeline(pipeline.get());

		for (size_t i = 0; i < attachments.ColorAttachmentCount(); i++)
		{
			auto attachment = attachments.colorAttachments[i];
			auto texture = attachment.resource;
			m_shader->SetTexture(attachment.resource->name.c_str(), attachment.resource);
		}

		for (size_t i = 0; i < attachments.BufferCount(); i++)
		{
			auto attachment = attachments.bufferAttachments[i];

			m_shader->SetBuffer(attachment.buffer->name.c_str(), attachment.buffer);
		}

		m_shader->GetPropertiesBlock()->ApplyProperties({ frameData.graphics, m_shader.get(), pipeline->vkDescriptorSets,computeCommands.get() });

		computeCommands->BindDescriptorSetCompute(pipeline->vkPipelineLayoutHandle, pipeline->vkDescriptorSets, pipeline->numDescriptorSets);

		computeCommands->Dispatch(m_groupX, m_groupY, m_groupZ);
	}
}
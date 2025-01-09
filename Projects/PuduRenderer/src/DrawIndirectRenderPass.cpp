#include "DrawIndirectRenderPass.h"

namespace Pudu
{
	void DrawIndirectRenderPass::Render(RenderFrameData& frameData)
	{
		auto commands = frameData.currentCommand;
		auto pipeline = GetPipeline({ .renderPass = this, .shader = m_material.shader.get(),.renderer = frameData.renderer, });

		UniformBufferObject ubo;
		ubo.ProjectionMatrix = frameData.camera->Projection.GetProjectionMatrix();
		ubo.viewMatrix = frameData.camera->GetViewMatrix();
		ubo.time = frameData.graphics->GetTime()->Time();

		frameData.globalPropertiesMaterial->GetPropertiesBlock()->ApplyProperties({ frameData.graphics, m_material.shader.get(), pipeline, commands.get() });
		m_material.GetPropertiesBlock()->ApplyProperties({ frameData.graphics, m_material.shader.get(), pipeline,commands.get() });
		commands->BindPipeline(pipeline);
		commands->BindDescriptorSet(pipeline->vkPipelineLayoutHandle, pipeline->vkDescriptorSets, pipeline->numDescriptorSets);

		commands->PushConstants(pipeline->vkPipelineLayoutHandle, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(UniformBufferObject), &ubo);
		commands->DrawIndirect(m_indirectBuffer.get(), m_offset, m_drawCount, m_stride);
	}
	DrawIndirectRenderPass* DrawIndirectRenderPass::SetIndirectBuffer(SPtr<GraphicsBuffer> buffer)
	{
		attachments.AddBufferAttachment(buffer);

		m_indirectBuffer = buffer;

		return this;
	}
	DrawIndirectRenderPass* DrawIndirectRenderPass::SetOffset(uint64_t offset)
	{
		m_offset = offset;
		return this;
	}
	DrawIndirectRenderPass* DrawIndirectRenderPass::SeStride(uint32_t stride)
	{
		m_stride = stride;
		return this;
	}
	DrawIndirectRenderPass* DrawIndirectRenderPass::SetDrawCount(uint32_t drawCount)
	{
		m_drawCount = drawCount;
		return this;
	}
	DrawIndirectRenderPass* DrawIndirectRenderPass::SetMaterial(Material& material)
	{
		m_material = material;
		return this;
	}
}

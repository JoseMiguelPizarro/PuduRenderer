#include "DrawIndirectRenderPass.h"

namespace Pudu
{
	void DrawIndirectRenderPass::Render(RenderFrameData& frameData)
	{
		auto commands = frameData.currentCommand;
		auto pipeline = GetPipeline({ .renderPass = this, .shader = m_shader.get() ,.renderer = frameData.renderer, });

		commands->BindPipeline(pipeline);
		commands->DrawIndirect(nullptr, m_offset, m_drawCount, m_stride);
	}
	void DrawIndirectRenderPass::SetIndirectBuffer(SPtr<GraphicsBuffer> buffer)
	{
		attachments.AddBufferAttachment(buffer);

		m_indirectBuffer = buffer;
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
	DrawIndirectRenderPass* DrawIndirectRenderPass::SetShader(SPtr<IShaderObject> shader)
	{
		m_shader = shader;
		return this;
	}
}

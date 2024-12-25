#pragma once
#include "FrameGraph/RenderPass.h"
#include "ComputeRenderPass.h"
#include "RenderFrameData.h"

namespace Pudu {
	class DrawIndirectRenderPass : public ComputeRenderPass
	{
	public:
		void Render(RenderFrameData& frameData) override;
		void SetIndirectBuffer(SPtr<GraphicsBuffer> buffer);
		DrawIndirectRenderPass* SetOffset(uint64_t offset);
		DrawIndirectRenderPass* SeStride(uint32_t stride);
		DrawIndirectRenderPass* SetDrawCount(uint32_t drawCount);
		DrawIndirectRenderPass* SetShader(SPtr<IShaderObject> shader);

	private:
		SPtr<GraphicsBuffer> m_indirectBuffer;
		uint64_t m_offset;
		uint32_t m_stride;
		uint32_t m_drawCount;
		SPtr<IShaderObject> m_shader;
	};
}


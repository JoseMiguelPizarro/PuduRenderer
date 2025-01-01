#pragma once
#include "FrameGraph/RenderPass.h"
#include "RenderFrameData.h"

namespace Pudu {
	class DrawIndirectRenderPass : public RenderPass
	{
	public:
		void Render(RenderFrameData& frameData) override;
		DrawIndirectRenderPass* SetIndirectBuffer(SPtr<GraphicsBuffer> buffer);
		DrawIndirectRenderPass* SetOffset(uint64_t offset);
		DrawIndirectRenderPass* SeStride(uint32_t stride);
		DrawIndirectRenderPass* SetDrawCount(uint32_t drawCount);
		DrawIndirectRenderPass* SetMaterial(Material& material);

	private:
		SPtr<GraphicsBuffer> m_indirectBuffer;
		Material m_material;
		uint64_t m_offset;
		uint32_t m_stride;
		uint32_t m_drawCount;
	};
}


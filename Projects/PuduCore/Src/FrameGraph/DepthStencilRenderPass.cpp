#include "FrameGraph/DepthStencilRenderPass.h"
#include "PuduRenderer.h"

namespace Pudu
{
	Pipeline* DepthStencilRenderPass::GetPipeline(RenderFrameData& frameData, DrawCall& drawcall)
	{
		return frameData.renderer->GetPipeline(drawcall);
	}
}

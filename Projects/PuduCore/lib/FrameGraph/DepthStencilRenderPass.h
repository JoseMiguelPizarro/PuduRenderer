#pragma once
#include "FrameGraphRenderPass.h"

namespace Pudu
{
	class DepthStencilRenderPass : public FrameGraphRenderPass
	{
		virtual Pipeline* GetPipeline(RenderFrameData& frameData, DrawCall& drawcall);
	};
}
#pragma once
#include <GPUCommands.h>
#include "Resources/Resources.h"
#include "RenderFrameData.h"
#include "DrawCall.h"

namespace Pudu
{
	class PuduGraphics;

	struct FrameGraphRenderPass
	{
		virtual Pipeline* GetPipeline(RenderFrameData& frameData, DrawCall& drawcall);
		virtual void PreRender(RenderFrameData& renderData) { }
		virtual void Render(RenderFrameData& frameData);
		virtual void OnResize(PuduGraphics& gpu, uint32_t newWidth, uint32_t newHeight) {}
	};

}

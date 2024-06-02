#pragma once
#include <GPUCommands.h>
#include "Resources/Resources.h"
#include "RenderFrameData.h"
#include "DrawCall.h"

namespace Pudu
{
	class PuduGraphics;

	class FrameGraphRenderPass
	{
	public:
		virtual Pipeline* GetPipeline(RenderFrameData& frameData, DrawCall& drawcall);
		virtual void PreRender(RenderFrameData& renderData) { }
		virtual void Render(RenderFrameData& frameData);
		virtual void OnResize(PuduGraphics& gpu, uint32_t newWidth, uint32_t newHeight) {}
		virtual void BeforeRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) {}
		virtual void AfterRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) {}
		virtual PipelineCreationData GetPipelineCreationData(RenderFrameData& frameData, DrawCall& drawcall);
		virtual void Initialize(PuduGraphics* gpu);
	};

}

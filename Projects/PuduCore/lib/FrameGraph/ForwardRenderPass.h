#pragma once
#include "FrameGraphRenderPass.h"

namespace Pudu 
{
	class ForwardRenderPass: public FrameGraphRenderPass
	{
		virtual void Render(RenderFrameData& renderData) override;
	};
}


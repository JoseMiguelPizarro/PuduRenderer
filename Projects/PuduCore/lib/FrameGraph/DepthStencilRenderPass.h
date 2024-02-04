#pragma once
#include "FrameGraphRenderPass.h"

namespace Pudu
{
	class DepthStencilRenderPass : FrameGraphRenderPass {
		void PreRender(VkCommandBuffer* commands, Scene* scene) override;
		void Render(VkCommandBuffer* commands, Scene* scene) override;
	};
}
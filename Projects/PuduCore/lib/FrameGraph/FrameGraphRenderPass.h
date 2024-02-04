#pragma once
#include "Scene.h"

namespace Pudu
{
	class PuduGraphics;

	struct FrameGraphRenderPass
	{
		virtual void AddUI() { }
		virtual void PreRender(VkCommandBuffer* gpu_commands, Scene* render_scene) { }
		virtual void Render(VkCommandBuffer* gpu_commands, Scene* render_scene) { }
		virtual void OnResize(PuduGraphics& gpu, uint32_t new_width, uint32_t new_height) {}
	};

}

#pragma once
#include <vector>

namespace Pudu
{
	class Frame;
	class FrameGraph;
	class Scene;
	class GPUCommands;
	class PuduGraphics;
	class PuduRenderer;

	struct RenderFrameData
	{
		uint32_t frameIndex;
		Frame* frame;
		FrameGraph* frameGraph;
		Scene* scene;
		GPUCommands* currentCommand;
		PuduGraphics* graphics;
		PuduRenderer* renderer;

		std::vector<VkCommandBuffer> commandsToSubmit;
	};
}

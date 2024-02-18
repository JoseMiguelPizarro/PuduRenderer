#pragma once
#include <vector>
#include <unordered_map>

namespace Pudu
{
	struct Frame;
	struct FrameGraph;
	class Scene;
	class GPUCommands;
	class PuduGraphics;
	class PuduRenderer;
	enum RenderPassType;
	class FrameGraphRenderPass;

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
		std::unordered_map<RenderPassType, FrameGraphRenderPass>* m_renderPassesByType;
	};
}

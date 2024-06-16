#pragma once
#include <vector>
#include <unordered_map>
#include "PuduCore.h"
#include <vulkan/vulkan_core.h>

namespace Pudu
{
	struct Frame;
	struct FrameGraph;
	struct RenderPass;
	struct DrawCall;
	class Scene;
	class GPUCommands;
	class PuduGraphics;
	class PuduRenderer;
	class Framebuffer;
	enum RenderPassType;
	class FrameGraphRenderPass;
	class Texture2d;
	class PuduApp;

	struct RenderFrameData
	{
		uint32_t frameIndex;
		Frame* frame;
		FrameGraph* frameGraph;
		Scene* scene;
		GPUCommands* currentCommand;
		PuduGraphics* graphics;
		PuduRenderer* renderer;
		SPtr<Texture2d> activeRenderTarget;
		RenderPass* currentRenderPass;
		DrawCall* currentDrawCall;
		Framebuffer* lastFrameBuffer;
		PuduApp* app;

		std::vector<VkCommandBuffer> commandsToSubmit;
		std::vector<GPUCommands*> computeCommandsToSubmit;
		std::unordered_map<RenderPassType, FrameGraphRenderPass*>* m_renderPassesByType;
	};
}

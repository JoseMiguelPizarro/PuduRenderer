#pragma once
#include <vector>
#include <unordered_map>
#include "PuduCore.h"
#include <vulkan/vulkan_core.h>

namespace Pudu
{
	struct Frame;
	class FrameGraph;
	class RenderPass;
	struct DrawCall;
	class Scene;
	class GPUCommands;
	class PuduGraphics;
	class PuduRenderer;
	struct Framebuffer;
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
		PuduApp* app;
		uint16_t width;
		uint16_t height;

		std::vector<VkCommandBuffer> commandsToSubmit;
		std::vector<GPUCommands*> computeCommandsToSubmit;
		std::unordered_map<RenderPassType, FrameGraphRenderPass*>* m_renderPassesByType;
	};
}

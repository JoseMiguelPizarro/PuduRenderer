#pragma once
#include <vector>
#include <unordered_map>
#include "PuduCore.h"
#include "GraphicsBuffer.h"
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
	class RenderPass;
	class Texture2d;
	class PuduApp;
	class Camera;
	class Pipeline;

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
		SPtr<RenderPass> currentRenderPass;
		DrawCall* currentDrawCall;
		Pipeline* currentPipeline;
		PuduApp* app;
		uint16_t width;
		uint16_t height;
		Camera* camera;
		SPtr<GraphicsBuffer> lightingBuffer;

		std::vector<VkCommandBuffer> commandsToSubmit;
		std::vector<GPUCommands*> computeCommandsToSubmit;
	};
}

#pragma once
#include <vector>
#include <unordered_map>
#include "PuduCore.h"
#include "GraphicsBuffer.h"
#include <vulkan/vulkan_core.h>
#include "PuduApp.h"

namespace Pudu
{
	struct Frame;
	class FrameGraph;
	class RenderPass;
	struct DrawCall;
	class Scene;
	class GPUCommands;
	class PuduGraphics;
	class Renderer;
	struct Framebuffer;
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
		SPtr<GPUCommands> currentCommand;
		PuduGraphics* graphics;
		Renderer* renderer;
		SPtr<RenderTexture> activeRenderTarget;
		SPtr<RenderPass> currentRenderPass;
		DrawCall* currentDrawCall;
		Pipeline* currentPipeline;
		PuduApp* app;
		uint16_t width;
		uint16_t height;
		Camera* camera;
		SPtr<GraphicsBuffer> lightingBuffer;
		SPtr<Material> globalPropertiesMaterial;
		uint32_t descriptorSetOffset;

		std::vector<VkCommandBuffer> commandsToSubmit;
		std::vector<SPtr<GPUCommands>> computeCommandsToSubmit;
	};
}

#pragma once
#include <array>
#include <glm/fwd.hpp>
#include <Viewport.h>
#include "Resources/Resources.h"
#include "PuduCore.h"
#include "Texture2D.h"

using namespace glm;

namespace Pudu
{
	class PuduGraphics;

	class GPUCommands
	{
	public:
		GPUCommands() = default;
		GPUCommands(VkCommandBuffer handle, PuduGraphics* gfx);
		void Reset();
		void BeginCommands();
		void EndCommands();
		void Clear(vec4 color);
		void ClearDepthStencil(float depth, float stencil);
		void AddImageBarrier(VkImage image, ResourceState oldState, ResourceState newState, u32 baseMipLevel, u32 mipCount, bool isDepth);
		void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		void SetViewport(Viewport const& viewport);
		void BindRenderPass(RenderPassHandle renderPassHandle, FramebufferHandle framebufferHandle);
		void EndCurrentRenderPass();
		void BindPipeline(Pipeline* pipeline);
		void BindDescriptorSet(VkDescriptorSet handle);
		void BindDescriptorSetCompute(VkPipelineLayout, VkDescriptorSet* handles, uint16_t handlesCount);
		void Blit(SPtr<Texture2d> source, SPtr<Texture2d> dst, VkImageLayout srcLayout, VkImageLayout dstLayout);
		void Dispatch(uint groupCountX, uint groupCountY, uint groupCountZ);
		VkCommandBuffer vkHandle;
		RenderPass* currentRenderPass;
		Framebuffer* currentFramebuffer;
		Pipeline* currentPipeline;
		bool HasRecordedCommand() { return m_hasRecordedCommand; }

	private:
		PuduGraphics* m_graphics = nullptr;
		std::array<VkClearValue, 2> m_clearValues{};
		bool m_hasRecordedCommand = false;
	};
}
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
		void Clear(vec4 color);
		void ClearDepthStencil(float depth, float stencil);
		void AddImageBarrier(VkImage image, ResourceState old_state, ResourceState new_state, u32 base_mip_level, u32 mip_count, bool is_depth);
		void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		void SetViewport(Viewport const& viewport);
		void BindRenderPass(RenderPassHandle renderPassHandle, FramebufferHandle framebufferHandle);
		void EndCurrentRenderPass();
		void BindPipeline(Pipeline* pipeline);
		void Blit(SPtr<Texture2d> source, SPtr<Texture2d> dst, VkImageLayout dstLayout);
		PuduGraphics* graphics;
		VkCommandBuffer vkHandle;
		RenderPass* currentRenderPass;
		Framebuffer* currentFramebuffer;
		Pipeline* currentPipeline;

	private:
		std::array<VkClearValue, 2> m_clearValues{};
	};
}
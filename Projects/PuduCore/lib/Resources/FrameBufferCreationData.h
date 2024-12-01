#pragma once
#include "Resources.h"
namespace Pudu
{
	class RenderPass;
	class RenderTexture;
	class Texture;

	struct FramebufferCreationData
	{
		GPUResourceHandle<RenderPass> renderPassHandle;

		uint16_t numRenderTargets = 0;

		GPUResourceHandle<Texture> outputTexturesHandle[K_MAX_IMAGE_OUTPUTS];
		GPUResourceHandle<Texture> depthStencilTextureHandle = { k_INVALID_HANDLE };

		uint16_t width = 0;
		uint16_t height = 0;

		float scaleX = 1.f;
		float scaleY = 1.f;
		uint8_t resize = 1;

		const char* name = nullptr;

		FramebufferCreationData& AddRenderTexture(GPUResourceHandle<Texture> resource);
		FramebufferCreationData& SetDepthStencilTexture(GPUResourceHandle<Texture> resource);
		FramebufferCreationData& SetScaling(float scale_x, float scale_y, uint8_t resize);
		FramebufferCreationData& SetName(const char* name);
	};
}

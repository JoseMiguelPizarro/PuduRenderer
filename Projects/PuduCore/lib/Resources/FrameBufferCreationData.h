#pragma once
#include "Resources.h"
namespace Pudu
{
	struct FramebufferCreationData
	{
		RenderPassHandle renderPassHandle;

		uint16_t numRenderTargets = 0;

		TextureHandle outputTexturesHandle[K_MAX_IMAGE_OUTPUTS];
		TextureHandle depthStencilTextureHandle = { k_INVALID_HANDLE };

		uint16_t width = 0;
		uint16_t height = 0;

		float scaleX = 1.f;
		float scaleY = 1.f;
		uint8_t resize = 1;

		const char* name = nullptr;

		FramebufferCreationData& Reset( );
		FramebufferCreationData& AddRenderTexture(TextureHandle texture);
		FramebufferCreationData& SetDepthStencilTexture(TextureHandle texture);
		FramebufferCreationData& SetScaling(float scale_x, float scale_y, uint8_t resize);
		FramebufferCreationData& SetName(const char* name);
	};
}

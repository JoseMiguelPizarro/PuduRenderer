#include "Resources/FrameBufferCreationData.h"

namespace Pudu
{
	FramebufferCreationData& FramebufferCreationData::Reset()
	{
		numRenderTargets = 0;
		name = nullptr;
		depthStencilTextureHandle.index = k_INVALID_HANDLE;

		resize = 0;
		scaleX = 1.0f;
		scaleY = 1.0f;

		return *this;
	}
	FramebufferCreationData& FramebufferCreationData::AddRenderTexture(GPUResourceHandle texture)
	{
		outputTexturesHandle[numRenderTargets++] = texture;

		return *this;
	}
	FramebufferCreationData& FramebufferCreationData::SetDepthStencilTexture(GPUResourceHandle texture)
	{
		depthStencilTextureHandle = texture;
		return *this;
	}
	FramebufferCreationData& FramebufferCreationData::SetScaling(float scale_x, float scale_y, uint8_t resize)
	{
		scaleX = scale_x;
		scaleY = scale_y;

		this->resize = resize;

		return *this;
	}
	FramebufferCreationData& FramebufferCreationData::SetName(const char* name)
	{
		this->name = name;

		return *this;
	}
}

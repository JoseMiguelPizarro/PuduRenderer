#include "Resources/FrameBufferCreationData.h"

namespace Pudu
{
	FrameBufferCreationData& FrameBufferCreationData::Reset()
	{
		numRenderTargets = 0;
		name = nullptr;
		depthStencilTexture.index = k_INVALID_HANDLE;

		resize = 0;
		scaleX = 1.0f;
		scaleY = 1.0f;

		return *this;
	}
	FrameBufferCreationData& FrameBufferCreationData::AddRenderTexture(TextureHandle texture)
	{
		outputTextures[numRenderTargets++] = texture;

		return *this;
	}
	FrameBufferCreationData& FrameBufferCreationData::SetDepthStencilTexture(TextureHandle texture)
	{
		depthStencilTexture = texture;
		return *this;
	}
	FrameBufferCreationData& FrameBufferCreationData::SetScaling(float scale_x, float scale_y, uint8_t resize)
	{
		scaleX = scale_x;
		scaleY = scale_y;

		this->resize = resize;

		return *this;
	}
	FrameBufferCreationData& FrameBufferCreationData::SetName(const char* name)
	{
		this->name = name;

		return *this;
	}
}

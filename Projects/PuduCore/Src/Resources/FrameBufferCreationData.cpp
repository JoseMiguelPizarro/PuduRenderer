#include "Resources/FrameBufferCreationData.h"

namespace Pudu
{
	
	FramebufferCreationData& FramebufferCreationData::AddRenderTexture(GPUResourceHandle<Texture> resource)
	{
		outputTexturesHandle[numRenderTargets++] = resource;

		return *this;
	}
	FramebufferCreationData& FramebufferCreationData::SetDepthStencilTexture(GPUResourceHandle<Texture> resource)
	{
		depthStencilTextureHandle = resource;
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

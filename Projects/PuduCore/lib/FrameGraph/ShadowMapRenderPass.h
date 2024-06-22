#pragma once
#include "FrameGraphRenderPass.h"
#include "PuduGraphics.h"

namespace Pudu
{
	class ShadowMapRenderPass :public FrameGraphRenderPass
	{
		void Initialize(PuduGraphics* gfx) override;
		

	private:
		SPtr<Texture2d> m_cubeMap;
		uint16_t m_resolution = 2048;
	};
}


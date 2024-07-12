#pragma once
#include "FrameGraph/RenderPass.h"
#include "PuduGraphics.h"

namespace Pudu
{
	class ShadowMapRenderPass :public RenderPass
	{
		const char* K_SHADOWMAP_NAME = "shadowmap";

		virtual void Initialize(PuduGraphics* gfx) override;
		virtual void PreRender(RenderFrameData& renderData) override;
		virtual void AfterRender(RenderFrameData& renderData) override;
		virtual RenderPassType GetRenderPassType() override;

	private:
		SPtr<Texture2d> m_shadowmap;
		uint16_t m_resolution = 2048;
		Camera m_renderCamera;
		Camera* m_previousCamera;
		glm::vec3 m_camOriginalPos;
		glm::vec3 m_camOriginalDir;
	};
}


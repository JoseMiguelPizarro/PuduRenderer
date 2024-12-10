#pragma once
#include "FrameGraph/RenderPass.h"
#include "PuduGraphics.h"
#include "RenderFrameData.h"

namespace Pudu
{
	class ShadowMapRenderPass :public RenderPass
	{
		const char* K_SHADOWMAP_NAME = "shadowmap";

		virtual void Initialize(PuduGraphics* gfx) override;
		virtual void PreRender(RenderFrameData& renderData) override;
		virtual void AfterRender(RenderFrameData& renderData) override;
		virtual void Render(RenderFrameData& frameData) override;


	private:

		void InitializeShadowMap(PuduGraphics* gfx);

		SPtr<Texture> m_shadowmap;
		uint16_t m_resolution = 2048;
		float m_depthConstantBias = 2.25f;
		float m_depthBiasSlope = 2.75;
		Camera m_renderCamera;
		Camera* m_previousCamera;
		glm::vec3 m_camOriginalPos;
		glm::vec3 m_camOriginalDir;
	};
}


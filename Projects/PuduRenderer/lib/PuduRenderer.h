#pragma once
#include "Scene.h"
#include "Renderer.h"
#include "PuduGraphics.h"
#include <FrameGraph/FrameGraph.h>
#include "ForwardRenderPass.h"
#include "ShadowMapRenderPass.h"
#include <PuduApp.h>
#include "ImguiRenderPass.h"

namespace Pudu
{
	class PuduRenderer :public Renderer
	{
	public:
		void SetSkyBox();
		SPtr<RenderTexture> GetDepthCopyRT() const;
		SPtr<RenderTexture> GetColorCopyRT() const;

	protected:
		void OnInit(PuduGraphics* graphics, PuduApp* app) override;
		void OnRender(RenderFrameData& data) override;
		void OnUploadCameraData(RenderFrameData& frameData) override;
		// In PuduRenderer.h

	private:
		void UpdateLightingBuffer(RenderFrameData& data) const;
		void UpdateGlobalConstantsBuffer(const RenderFrameData& frame) const;
		void InitLightingBuffer(PuduGraphics* graphics);
		void InitConstantsBuffer(PuduGraphics* graphics);

		SPtr<RenderTexture> m_depthCopyRT;
		SPtr<RenderTexture> m_colorCopyRT;
		SPtr<RenderPass> m_depthRenderPass;
		SPtr<RenderPass> m_forwardRenderPass;
		SPtr<RenderPass> m_shadowMapRenderPass;
		SPtr<RenderPass> m_postProcessingRenderPass;
		SPtr<ImguiRenderPass> m_imguiRenderPass;
		SPtr<GraphicsBuffer> m_lightingBuffer;
		SPtr<GraphicsBuffer> m_globalConstantsBuffer;
		SPtr<Material> m_globalPropertiesMaterial;
		SPtr<DescriptorSetLayoutsCollection> m_globalDescriptorSetLayouts;
	};
}


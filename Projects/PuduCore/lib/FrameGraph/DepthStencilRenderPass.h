#pragma once
#include "FrameGraphRenderPass.h"

namespace Pudu
{
	class DepthStencilRenderPass : public FrameGraphRenderPass
	{
	public:

		const std::filesystem::path K_DepthShaderPath = "Shaders/depth.ver";
		virtual Pipeline* GetPipeline(RenderFrameData& frameData, DrawCall& drawcall) override;
		virtual void Initialize(PuduGraphics* graphics) override;
		virtual void PreRender(RenderFrameData& renderData) override;
		virtual void BeforeRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) override;
		virtual void AfterRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) override;

	private:
		SPtr<Shader> m_depthShader;
		SPtr<Material> m_depthMaterial;
	};
}
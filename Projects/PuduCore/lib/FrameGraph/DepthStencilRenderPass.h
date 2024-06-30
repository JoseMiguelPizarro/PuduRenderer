#pragma once
#include "FrameGraph/RenderPass.h"

namespace Pudu
{
	class DepthStencilRenderPass : public RenderPass
	{
	public:

		const std::filesystem::path K_DepthShaderPath = "Shaders/depth.vert";
		Pipeline* GetPipeline(RenderFrameData& frameData, DrawCall& drawcall) override;
		void Initialize(PuduGraphics* gpu) override;
		void PreRender(RenderFrameData& renderData) override;
		void BeforeRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) override;
		void AfterRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) override;
		RenderPassType GetRenderPassType() override { return RenderPassType::DepthPrePass; };

	private:
		SPtr<Shader> m_depthShader;
		SPtr<Material> m_depthMaterial;
	};
}
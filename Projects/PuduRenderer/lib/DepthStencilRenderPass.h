#pragma once
#include "FrameGraph/RenderPass.h"

namespace Pudu
{
	class DepthPrepassRenderPass : public RenderPass
	{
	public:
		const std::filesystem::path K_DepthShaderPath = "depth.shader.slang";
		void Initialize(PuduGraphics* gpu) override;
		void BeforeRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) override;
		void AfterRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall) override;

	private:
		SPtr<Shader> m_depthShader;
		SPtr<Material> m_depthMaterial;
	};
}
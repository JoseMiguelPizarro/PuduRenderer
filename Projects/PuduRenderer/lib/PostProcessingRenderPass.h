#pragma once

#include "FrameGraph/RenderPass.h"
#include "vertex.h"
#include "PuduGraphics.h"
#include "RenderFrameData.h"


namespace Pudu
{


	class PostProcessingRenderPass :public RenderPass
	{
	public:
		void Initialize(PuduGraphics* gpu) override;
		void Render(RenderFrameData& renderData) override;
		void PreRender(RenderFrameData& renderData) override;
		void SetExposure(float exposure);

	private:
		SPtr<Mesh> m_quadMesh;
		SPtr<Shader> m_postProcessingShader;
		SPtr<RenderTexture> m_screenColor;
		SPtr<Material> m_material;

		std::filesystem::path m_shaderPath = "postprocessing.shader.slang";
	};
}
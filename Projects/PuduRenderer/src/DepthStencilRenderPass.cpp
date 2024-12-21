#include "PuduCore.h"
#include "DepthStencilRenderPass.h"
#include "FileManager.h"
#include <memory>
#include "PuduRenderer.h"
#include "DepthStencilRenderPass.h"

namespace Pudu
{
	void DepthPrepassRenderPass::Initialize(PuduGraphics* graphics)
	{
		m_depthShader = graphics->CreateShader(K_DepthShaderPath, "Depth");
		auto m = Material();
		m.Shader = m_depthShader;
		m.Name = "DepthPassMaterial";
		m_depthMaterial = std::make_shared<Material>(m);
	}

	void DepthPrepassRenderPass::PreRender(RenderFrameData& renderData)
	{
	}

	void DepthPrepassRenderPass::BeforeRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall)
	{
		drawcall.SetReplacementMaterial(m_depthMaterial);
	}

	void DepthPrepassRenderPass::AfterRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall)
	{
		drawcall.SetReplacementMaterial(nullptr);
	}
}

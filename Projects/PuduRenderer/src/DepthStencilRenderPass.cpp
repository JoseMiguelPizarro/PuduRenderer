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
		m_depthMaterial = graphics->Resources()->AllocateMaterial();
		m_depthMaterial->SetShader(m_depthShader);
		m_depthMaterial->name = "DepthPassMaterial";
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

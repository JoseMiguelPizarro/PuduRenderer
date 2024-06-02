#include "PuduCore.h"
#include "FrameGraph/DepthStencilRenderPass.h"
#include "FileManager.h"
#include <memory>
#include "PuduRenderer.h"
#include "FrameGraph/DepthStencilRenderPass.h"

namespace Pudu
{
	void DepthStencilRenderPass::Initialize(PuduGraphics* graphics)
	{
		auto shaderFile = FileManager::LoadShader(K_DepthShaderPath);
		m_depthShader = graphics->CreateShader(fs::path(), K_DepthShaderPath, "Depth");
		auto m = Material();
		m.Shader = m_depthShader;
		m.Name = "DepthPassMaterial";
		m_depthMaterial = std::make_shared<Material>(m);
	}

	void DepthStencilRenderPass::PreRender(RenderFrameData& renderData)
	{
	}

	void DepthStencilRenderPass::BeforeRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall)
	{
		drawcall.SetReplacementMaterial(m_depthMaterial);
	}

	void DepthStencilRenderPass::AfterRenderDrawcall(RenderFrameData& frameData, DrawCall& drawcall)
	{
		drawcall.SetReplacementMaterial(nullptr);
	}

	Pipeline* DepthStencilRenderPass::GetPipeline(RenderFrameData& frameData, DrawCall& drawcall)
	{
		return frameData.renderer->GetOrCreatePipeline(frameData, RenderPassType::DepthPrePass);
	}
}

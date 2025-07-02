#include "ShadowMapRenderPass.h"

#include "Renderer.h"
#include "Resources/Resources.h"

namespace Pudu
{
    void ShadowMapRenderPass::Initialize(PuduGraphics* gfx)
    {
        m_renderCamera = {};
        m_depthShader = gfx->CreateShader(K_DepthShaderPath, "Depth");
        m_depthMaterial = gfx->CreateMaterial();
        m_depthMaterial->SetShader(m_depthShader);
        m_depthMaterial->name = "ShadowmapMaterial";
    }

    void ShadowMapRenderPass::PreRender(RenderFrameData& renderData)
    {
        RenderPass::PreRender(renderData);

        m_previousCamera = renderData.renderer->GetRenderCamera();
        auto dirLight = renderData.scene->directionalLight;
        auto dir = dirLight->Direction();
        m_renderCamera.Transform.SetForward(dir, {0, 1, 0});
        m_renderCamera.Transform.SetLocalPosition(dirLight->GetTransform().GetLocalPosition());
        m_renderCamera.Projection = renderData.scene->directionalLight->Projection;

        renderData.renderer->SetRenderCamera(&m_renderCamera);
    }

    void ShadowMapRenderPass::AfterRender(RenderFrameData& renderData)
    {
        renderData.renderer->SetRenderCamera(m_previousCamera);
    }

    void ShadowMapRenderPass::Render(RenderFrameData& frameData)
    {
        frameData.currentCommand->SetDepthBias(m_depthBiasSlope, m_depthConstantBias);
        RenderPass::Render(frameData);
    }

    void ShadowMapRenderPass::SetDepthBiasSlope(float slope)
    {
        m_depthBiasSlope = slope;
    }

    void ShadowMapRenderPass::SetDepthConstantBias(float bias)
    {
        m_depthConstantBias = bias;
    }
}

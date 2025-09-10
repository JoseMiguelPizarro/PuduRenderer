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
        auto dirLight = renderData.directionalLight;
        if (dirLight == nullptr)
            return;

        RenderPass::PreRender(renderData);

        auto shadowMap = attachments.depthAttachments[0].resource;

        m_previousCamera = renderData.renderer->GetRenderCamera();


        auto dir = dirLight->Direction();
        m_renderCamera.Transform.SetForward(dir, {0, 1, 0});
        m_renderCamera.Transform.SetLocalPosition(dirLight->GetTransform().GetLocalPosition());

        auto projection = renderData.directionalLight->GetProjection();
        m_renderCamera.Projection = projection;

        renderData.renderer->SetRenderCamera(&m_renderCamera);
    }

    void ShadowMapRenderPass::AfterRender(RenderFrameData& renderData)
    {
        if (renderData.directionalLight == nullptr)
            return;

        renderData.renderer->SetRenderCamera(m_previousCamera);
    }

    void ShadowMapRenderPass::Render(RenderFrameData& frameData)
    {
        if (frameData.directionalLight == nullptr)
            return;

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

    Viewport ShadowMapRenderPass::GetViewport(RenderFrameData& frameData) const
    {
        auto shadowMap = attachments.depthAttachments[0].resource;
        Viewport viewport;
        viewport.rect = {0, 0, static_cast<u16>(shadowMap->width), static_cast<u16>(shadowMap->height)};
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        return viewport;
    }
}

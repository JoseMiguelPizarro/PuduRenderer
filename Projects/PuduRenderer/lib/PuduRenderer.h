#pragma once
#include "Scene.h"
#include "Renderer.h"
#include "PuduGraphics.h"
#include <FrameGraph/FrameGraph.h>
#include "ForwardRenderPass.h"
#include "ShadowMapRenderPass.h"
#include <PuduApp.h>
#include "ImguiRenderPass.h"
#include "OmnidirectionalShadowmapRenderPass.h"
#include "PostProcessingRenderPass.h"

namespace Pudu
{
    class PuduRenderer : public Renderer
    {
    public:
        void SetSkyBox(SPtr<Texture> skybox, bool initIBL = true);
        SPtr<Texture> GetSkybox();
        SPtr<Texture> GetEnvMap();
        SPtr<Texture> GetBRDF_LUT();
        SPtr<Texture> GetIBL();
        SPtr<Texture> GetIBLDiffuse();
        SPtr<RenderTexture> GetDepthCopyRT() const;
        SPtr<RenderTexture> GetColorCopyRT() const;
        SPtr<RenderTexture> GetOmnidirectionalShadowmapRT() const;
        void SetDebugMode(Renderer::Debug mode);
        void SetRoughnessScale(float scale);
        void EnablePostProcessing(bool enable) const;
        void SetGamma(float gamma);
        void EnableToneMapping(bool enable) const;
        void SetExposure(float value) const;
        void SetShadowBias(float slope, float bias);

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
        void InitBRDF_LUT(PuduGraphics* gfx);
        void InitIBL(PuduGraphics* gfx, SPtr<Texture> envMap);

        float m_gamma = 2.2;

        SPtr<Texture> m_BRDF_LUT;
        SPtr<Texture> m_skybox;
        SPtr<Texture> m_envMap;
        SPtr<Texture> m_IBL;
        SPtr<Texture> m_IBL_SpecularCube;
        SPtr<Texture> m_IBL_DiffuseCube;
        SPtr<RenderTexture> m_depthCopyRT;
        SPtr<RenderTexture> m_colorCopyRT;
        SPtr<RenderTexture> m_omnidirectionalShadowRT;
        SPtr<RenderPass> m_depthRenderPass;
        SPtr<RenderPass> m_forwardRenderPass;
        SPtr<ShadowMapRenderPass> m_shadowMapRenderPass;
        SPtr<OmnidirectionalShadowmapRenderPass> m_omnidirectionalShadowMapRenderPass;
        SPtr<PostProcessingRenderPass> m_postProcessingRenderPass;
        SPtr<ImguiRenderPass> m_imguiRenderPass;
        SPtr<GraphicsBuffer> m_lightingBuffer;
        SPtr<GraphicsBuffer> m_globalConstantsBuffer;
        SPtr<GraphicsBuffer> m_omnidirectionalLightBuffer;
        SPtr<Material> m_globalPropertiesMaterial;
        SPtr<DescriptorSetLayoutsCollection> m_globalDescriptorSetLayouts;
    };
}

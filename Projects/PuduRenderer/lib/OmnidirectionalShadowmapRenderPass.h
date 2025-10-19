//
// Created by Hojaverde on 9/12/2025.
//

#pragma once
#include "FrameGraph/RenderPass.h"

namespace Pudu
{
    struct OmnidirectionalShadowmapData
    {
        float4 lightCount;
        float4x4 shadowMatrix[128]; //FOR NOW HARD CODED TO 128
    };


    class OmnidirectionalShadowmapRenderPass:public RenderPass
    {
    public:
        void OnCreate(PuduGraphics* gfx) override;
        void PreRender(RenderFrameData& renderData) override;
        void Render(RenderFrameData& renderData) override;
        void SetBuffer(const SPtr<GraphicsBuffer>& buffer);

    private:
        SPtr<GraphicsBuffer> m_omnidirectionalBuffer;
        SPtr<Shader> m_omnidirectionalShader;
        SPtr<Material> m_omnidirectionalMaterial;
        OmnidirectionalShadowmapData m_data;
    };
}



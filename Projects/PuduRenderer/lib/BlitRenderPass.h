//
// Created by Hojaverde on 2/9/2025.
//

#pragma once
#include "FrameGraph/RenderPass.h"


namespace Pudu
{
    class BlitRenderPass:public RenderPass {
    public:
        void Render(RenderFrameData& frameData) override;
        void SetBlitTargets(const SPtr<RenderTexture>& src, const SPtr<RenderTexture>& dst);

    private:
        SPtr<RenderTexture> m_src;
        SPtr<RenderTexture> m_dst;
    };
}





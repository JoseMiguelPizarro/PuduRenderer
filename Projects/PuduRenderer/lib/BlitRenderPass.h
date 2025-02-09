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
        void SetTargets(const SPtr<Texture>& src, const SPtr<Texture>& dst);

    private:
        SPtr<Texture> m_src;
        SPtr<Texture> m_dst;
    };
}





//
// Created by Hojaverde on 2/9/2025.
//

#pragma once
#include "FrameGraph/RenderPass.h"


namespace Pudu
{
    class BlitRenderPass:public RenderPass {
    public:
        void Render(RenderFrameData& renderData) override;
        void SetBlitTargets(const SPtr<RenderTexture>& src, const SPtr<RenderTexture>& dst);
        void PreRender(RenderFrameData& renderData) override;
        void AfterRender(RenderFrameData& renderData) override;

    private:
        SPtr<RenderTexture> m_src;
        SPtr<RenderTexture> m_dst;
        VkImageLayout m_srcLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    };
}





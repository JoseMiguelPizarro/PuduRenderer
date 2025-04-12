//
// Created by Hojaverde on 2/9/2025.
//

#include "BlitRenderPass.h"

#include <utility>

#include "RenderFrameData.h"

namespace Pudu
{
    void BlitRenderPass::SetBlitTargets(const SPtr<RenderTexture>& src, const SPtr<RenderTexture>& dst)
    {
        m_src = src;
        m_dst = dst;
        AddColorAttachment(std::dynamic_pointer_cast<RenderTexture>(src), CopySrc, Load);
        AddColorAttachment(std::dynamic_pointer_cast<RenderTexture>(dst), CopyDst, DontCare);
    }

    void BlitRenderPass::PreRender(RenderFrameData& renderData)
    {
        const auto command = renderData.currentCommand;

        m_srcLayout = m_src->GetImageLayout();

        //  command->TransitionTextureLayout(m_src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        //command->TransitionTextureLayout(m_dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    }

    void BlitRenderPass::AfterRender(RenderFrameData& renderData)
    {
        //TODO: What do we do with the source image? we'll need a way of knowing its previous copy layout so we can put it back to it

        auto command = renderData.currentCommand;
      //  command->TransitionTextureLayout(m_src, m_srcLayout);
        //     command->TransitionTextureLayout(m_dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        renderData.currentCommand->Blit(m_src, m_dst, m_src->GetImageLayout(),
                                        m_dst->GetImageLayout());
    }

    void BlitRenderPass::Render(RenderFrameData& renderData)
    {
    }
}

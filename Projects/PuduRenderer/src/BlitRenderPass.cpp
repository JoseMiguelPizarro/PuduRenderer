//
// Created by Hojaverde on 2/9/2025.
//

#include "BlitRenderPass.h"

#include <utility>

#include "RenderFrameData.h"

namespace Pudu
{
    void BlitRenderPass::SetTargets(const SPtr<Texture>& src, const SPtr<Texture>& dst)
    {
        m_src = src;
        m_dst = dst;
        AddColorAttachment(std::dynamic_pointer_cast<RenderTexture>(src), Write, DontCare);
        AddColorAttachment(std::dynamic_pointer_cast<RenderTexture>(dst), Read, DontCare);
    }

    void BlitRenderPass::Render(RenderFrameData& frameData)
    {
        const auto command = frameData.currentCommand;

        //TODO: We are assuming color attachment, we need to find a way to track the layouts
        command->TransitionImageLayout(m_src->vkImageHandle, m_src->format,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        command->TransitionImageLayout(m_dst->vkImageHandle, m_dst->format,VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        command->Blit(m_src,m_dst,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        command->TransitionImageLayout(m_src->vkImageHandle,m_src->format,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        command->TransitionImageLayout(m_dst->vkImageHandle,m_dst->format,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        //TODO: What do we do with the source image? we'll need a way of knowing its previous copy layout so we can put it back to it
    }
}





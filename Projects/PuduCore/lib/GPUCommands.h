#pragma once
#include <array>
#include <glm/fwd.hpp>
#include <Viewport.h>
#include "Resources/Resources.h"
#include "PuduCore.h"
#include "Texture2D.h"
#include "Resources/GPUResource.h"

using namespace glm;

namespace Pudu
{
    class PuduGraphics;
    class RenderPass;
    class Pipeline;

    class GPUCommands : public GPUResource<GPUCommands>
    {
        struct CreationData
        {
            VkCommandPool pool;
            uint32_t count;
        };

    public:
        GPUCommands() = default;
        GPUCommands(VkCommandBuffer handle, PuduGraphics* gfx);
        void Reset();
        void BeginCommands();
        void EndCommands();
        void Clear(vec4 color);
        void ClearDepthStencil(float depth, float stencil);
        void AddImageBarrier(VkImage image, ResourceState oldState, ResourceState newState, u32 baseMipLevel,
                             u32 mipCount, bool isDepth);
        void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        void SetViewport(Viewport const& viewport);

        /// <summary>
        /// Beging dynamic rendering renderpass
        /// </summary>
        void BegingRenderingPass(const VkRenderingInfo& renderInfo);
        void EndRenderingPass();

        void BegingRenderPass(const VkRenderPassBeginInfo& renderInfo);
        void EndRenderPass();
        void BindPipeline(Pipeline* pipeline);
        void BindDescriptorSet(VkPipelineLayout pipelineLayout, VkDescriptorSet* handles, uint16_t handlesCount);
        void BindDescriptorSetCompute(VkPipelineLayout, VkDescriptorSet* handles, uint16_t handlesCount);
        void Blit(SPtr<Texture> source, SPtr<Texture> dst, VkImageLayout srcLayout, VkImageLayout dstLayout);
        void Dispatch(uint groupCountX, uint groupCountY, uint groupCountZ);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                                   VkImageSubresourceRange* range = nullptr);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
                               std::vector<VkBufferImageCopy2>* regions = nullptr);
        void SetDepthBias(float slopeBias, float constantBias);

        VkCommandBuffer vkHandle;
        Framebuffer* currentFramebuffer;
        Pipeline* currentPipeline;
        bool HasRecordedCommand() { return m_hasRecordedCommand; }

    private:
        friend class PuduGraphics;
        PuduGraphics* m_graphics = nullptr;
        std::array<VkClearValue, 2> m_clearValues{};
        bool m_hasRecordedCommand = false;
    };
}

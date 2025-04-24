#pragma once
#include <array>
#include <glm/fwd.hpp>
#include <Viewport.h>
#include "Resources/Resources.h"
#include "PuduCore.h"
#include "Texture2D.h"
#include "Resources/GPUResource.h"
#include "Mesh.h"
#include "Enums/ResourceUsage.h"

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
        void ImageBarrier(const VkImageMemoryBarrier2* barrier) const;
        void AddImageBarrier(VkImage image, ResourceUsage oldUsage, ResourceUsage newUsage, u32 baseMipLevel,
                             u32 mipCount, bool isDepth) const;
        void AddMemoryBarrier(VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask,
                              VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask);
        void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        void SetViewport(Viewport const& viewport);
        void PushConstants(VkPipelineLayout pipelineLayout, VkPipelineStageFlags stagesFlag, uint32_t offset,
                           uint32_t size, void* data);
        void PushDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout pipelineLayout, uint32_t set,
                                uint32_t writeCount, const VkWriteDescriptorSet* writes);

        void UploadBufferData(GraphicsBuffer* buffer, const byte* data, Size size, Size offset = 0) const;
        void BufferBarrier(GraphicsBuffer* buffer, Size size, Size offset, VkAccessFlags srcAccessMask,
                           VkAccessFlags dstAccessMask, u32 srcQueueIndex, u32 dstQueueIndex);
        /// <summary>
        /// Beging dynamic rendering renderpass
        /// </summary>
        void BegingRenderingPass(const VkRenderingInfo& renderInfo);
        void EndRenderingPass();

        void BegingRenderPass(const VkRenderPassBeginInfo& renderInfo);
        void EndRenderPass();
        void BindPipeline(const Pipeline* pipeline);
        void BindDescriptorSet(VkPipelineLayout pipelineLayout, VkDescriptorSet* handles, uint16_t handlesCount,
                               uint32_t offset = 0);
        void BindDescriptorSetCompute(VkPipelineLayout, VkDescriptorSet* handles, uint16_t handlesCount);
        void Blit(SPtr<Texture> source, SPtr<Texture> dst, VkImageLayout srcLayout, VkImageLayout dstLayout);
        void Blit(SPtr<Texture> source, SPtr<Texture> dst, VkFilter filter, VkImageLayout srcLayout, VkImageLayout dstLayout,
          VkImageBlit2* regions, Size regionCount = 1) const;
        void Blit(Texture* source, Texture* dst, VkFilter filter, VkImageLayout srcLayout, VkImageLayout dstLayout,
                  VkImageBlit2* regions, Size regionCount = 1) const;
        void Dispatch(uint groupCountX, uint groupCountY, uint groupCountZ);
        void DispatchIndirect(GraphicsBuffer* paramsBuffer, uint64_t offset);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                                   VkImageSubresourceRange* range = nullptr);

        void TransitionTextureLayout(const SPtr<Texture>& texture, VkImageLayout layout, VkImageSubresourceRange* range = nullptr);
        void TransitionTextureLayout(Texture* texture, VkImageLayout layout, VkImageSubresourceRange* range = nullptr);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
                               std::vector<VkBufferImageCopy2>* regions = nullptr);
        void SetDepthBias(float slopeBias, float constantBias);
        void BindMesh(Mesh* mesh);
        void DrawIndexed(uint32_t indexCount,
                         uint32_t instanceCount = 1,
                         uint32_t firstIndex = 0,
                         int32_t vertexOffset = 0,
                         uint32_t firstInstance = 0);

        void DrawIndirect(GraphicsBuffer* buffer, uint64_t offset, uint32_t drawCount, uint32_t stride);


        VkCommandBuffer vkHandle;
        Framebuffer* currentFramebuffer;
        bool HasRecordedCommand() { return m_hasRecordedCommand; }

    private:
        friend class PuduGraphics;
        PuduGraphics* m_graphics = nullptr;
        std::array<VkClearValue, 2> m_clearValues{};
        bool m_hasRecordedCommand = false;
    };
}

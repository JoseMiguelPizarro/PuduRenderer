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
        /// <summary>
        /// Creates a new GPU command buffer with specified handle and graphics context
        /// </summary>
        /// <param name="handle">Vulkan command buffer handle</param>
        /// <param name="gfx">Pointer to graphics context</param>
        GPUCommands(VkCommandBuffer handle, PuduGraphics* gfx);

        /// <summary>
        /// Resets the command buffer to initial state
        /// </summary>
        void Reset();

        /// <summary>
        /// Begins recording commands to the command buffer
        /// </summary>
        void BeginCommands();

        /// <summary>
        /// Ends command buffer recording
        /// </summary>
        void EndCommands();

        /// <summary>
        /// Clears render target with specified color
        /// </summary>
        /// <param name="color">Clear color value</param>
        void Clear(vec4 color);

        /// <summary>
        /// Clears depth and stencil buffers
        /// </summary>
        /// <param name="depth">Depth clear value</param>
        /// <param name="stencil">Stencil clear value</param>
        void ClearDepthStencil(float depth, float stencil);

        /// <summary>
        /// Adds an image memory barrier
        /// </summary>
        /// <param name="barrier">Pointer to image barrier structure</param>
        void ImageBarrier(const VkImageMemoryBarrier2* barrier) const;

        /// <summary>
        /// Adds an image resource barrier
        /// </summary>
        /// <param name="image">Image handle</param>
        /// <param name="oldUsage">Previous resource usage</param>
        /// <param name="newUsage">New resource usage</param>
        /// <param name="baseMipLevel">Base mip level</param>
        /// <param name="mipCount">Number of mip levels</param>
        /// <param name="isDepth">Whether image is depth format</param>
        void AddImageBarrier(VkImage image, ResourceUsage oldUsage, ResourceUsage newUsage, u32 baseMipLevel,
                             u32 mipCount, bool isDepth) const;

        /// <summary>
        /// Adds a memory barrier
        /// </summary>
        /// <param name="srcStageMask">Source pipeline stage flags</param>
        /// <param name="dstStageMask">Destination pipeline stage flags</param>
        /// <param name="srcAccessMask">Source access flags</param>
        /// <param name="dstAccessMask">Destination access flags</param>
        void AddMemoryBarrier(VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask,
                              VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask);

        /// <summary>
        /// Sets the scissor rectangle
        /// </summary>
        /// <param name="x">X coordinate</param>
        /// <param name="y">Y coordinate</param>
        /// <param name="width">Width of scissor region</param>
        /// <param name="height">Height of scissor region</param>
        void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        /// <summary>
        /// Sets the viewport
        /// </summary>
        /// <param name="viewport">Viewport settings</param>
        void SetViewport(Viewport const& viewport);

        /// <summary>
        /// Pushes constants to the GPU pipeline
        /// </summary>
        /// <param name="pipelineLayout">Pipeline layout handle</param>
        /// <param name="stagesFlag">Pipeline stage flags</param>
        /// <param name="offset">Offset in bytes</param>
        /// <param name="size">Size in bytes</param>
        /// <param name="data">Pointer to constant data</param>
        void PushConstants(VkPipelineLayout pipelineLayout, VkPipelineStageFlags stagesFlag, uint32_t offset,
                           uint32_t size, void* data);

        /// <summary>
        /// Pushes descriptor sets to the GPU pipeline
        /// </summary>
        /// <param name="pipelineBindPoint">Pipeline bind point</param>
        /// <param name="pipelineLayout">Pipeline layout handle</param>
        /// <param name="set">Descriptor set index</param>
        /// <param name="writeCount">Number of descriptor writes</param>
        /// <param name="writes">Array of descriptor write operations</param>
        void PushDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout pipelineLayout, uint32_t set,
                                uint32_t writeCount, const VkWriteDescriptorSet* writes);

        void UploadBufferData(GraphicsBuffer* buffer, const byte* data, Size size, Size offset = 0) const;
        void BufferBarrier(GraphicsBuffer* buffer, Size size, Size offset, VkAccessFlags srcAccessMask,
                           VkAccessFlags dstAccessMask, u32 srcQueueIndex, u32 dstQueueIndex);
        /// <summary>
        /// Begings dynamic rendering renderpass
        /// </summary>
        void BegingRenderingPass(const VkRenderingInfo& renderInfo);

        /// <summary>
        /// Ends dynamic rendering pass
        /// </summary>
        void EndRenderingPass();


        /// @deprecated Use BegingRenderingPass instead. Only DynamicRendering planned to be supported.
        [[deprecated("Use BegingRenderingPass instead")]]
        void BegingRenderPass(const VkRenderPassBeginInfo& renderInfo);
        /// @deprecated Use EndRenderingPass instead. Only DynamicRendering planned to be supported.
        [[deprecated("Use EndRenderingPass instead")]]
        void EndRenderPass();

        void BindPipeline(const Pipeline* pipeline);
        void BindDescriptorSet(VkPipelineLayout pipelineLayout, VkDescriptorSet* handles, uint16_t handlesCount,
                               uint32_t offset = 0);
        /// <summary>
        /// Binds descriptor sets for compute pipeline
        /// </summary>
        /// <param name="pipelineLayout">Pipeline layout handle</param>
        /// <param name="handles">Array of descriptor set handles</param>
        /// <param name="handlesCount">Number of descriptor sets</param>
        void BindDescriptorSetCompute(VkPipelineLayout pipelineLayout, VkDescriptorSet* handles, uint16_t handlesCount);


        /// <summary>
        /// Copies source texture to destination texture using default linear filter
        /// </summary>
        /// <param name="src">Source texture</param>
        /// <param name="dst">Destination texture</param>
        void Blit(const SPtr<Texture>& src, const SPtr<Texture>& dst);

        ///<summary>Blits src texture to dst texture using a linear filter through all mip levels</summary>
        void BlitMips(const SPtr<Texture>& src, const SPtr<Texture>& dst);
        /// <summary>
        /// Copies source texture to destination texture using specified filter
        /// </summary>
        /// <param name="src">Source texture</param>
        /// <param name="dst">Destination texture</param>
        /// <param name="filter">Vulkan filter to use during the blit operation</param>
        void Blit(const SPtr<Texture>& src, const SPtr<Texture>& dst, VkFilter filter);

        /// <summary>
        /// Copies source texture to destination texture using specified filter and blit region
        /// </summary>
        /// <param name="src">Source texture</param>
        /// <param name="dst">Destination texture</param>
        /// <param name="filter">Vulkan filter to use during the blit operation</param>
        /// <param name="blitRegion">Specific region to blit</param>
        void Blit(const SPtr<Texture>& src, const SPtr<Texture>& dst, VkFilter filter, VkImageBlit2* blitRegion);

        /// <summary>
        /// Copies source texture to destination texture using specified layouts
        /// </summary>
        /// <param name="src">Source texture</param>
        /// <param name="dst">Destination texture</param>
        /// <param name="srcLayout">Source texture layout</param>
        /// <param name="dstLayout">Destination texture layout</param>
        void Blit(const SPtr<Texture>& src, const SPtr<Texture>& dst, VkImageLayout srcLayout, VkImageLayout dstLayout);

        /// <summary>
        /// Copies source texture to destination texture with full control over filter, layouts and regions
        /// </summary>
        /// <param name="src">Source texture</param>
        /// <param name="dst">Destination texture</param>
        /// <param name="filter">Vulkan filter to use during the blit operation</param>
        /// <param name="srcLayout">Source texture layout</param>
        /// <param name="dstLayout">Destination texture layout</param>
        /// <param name="regions">Array of blit regions</param>
        /// <param name="regionCount">Number of regions to blit</param>
        void Blit(const SPtr<Texture>& src, const SPtr<Texture>& dst, VkFilter filter, VkImageLayout srcLayout,
                  VkImageLayout dstLayout,
                  VkImageBlit2* regions, Size regionCount = 1) const;

        /// <summary>
        /// Raw pointer version of Blit operation with full control over filter, layouts and regions
        /// </summary>
        /// <param name="src">Source texture pointer</param>
        /// <param name="dst">Destination texture pointer</param>
        /// <param name="filter">Vulkan filter to use during the blit operation</param>
        /// <param name="srcLayout">Source texture layout</param>
        /// <param name="dstLayout">Destination texture layout</param>
        /// <param name="regions">Array of blit regions</param>
        /// <param name="regionCount">Number of regions to blit</param>
        void Blit(const Texture* src, const Texture* dst, VkFilter filter, VkImageLayout srcLayout, VkImageLayout dstLayout,
                  VkImageBlit2* regions, Size regionCount = 1) const;
        /// <summary>
        /// Dispatches compute work
        /// </summary>
        /// <param name="groupCountX">Number of work groups in X dimension</param>
        /// <param name="groupCountY">Number of work groups in Y dimension</param>
        /// <param name="groupCountZ">Number of work groups in Z dimension</param>
        void Dispatch(uint groupCountX, uint groupCountY, uint groupCountZ);

        /// <summary>
        /// Dispatches compute work with parameters from a buffer
        /// </summary>
        /// <param name="paramsBuffer">Buffer containing dispatch parameters</param>
        /// <param name="offset">Offset into the buffer</param>
        void DispatchIndirect(const GraphicsBuffer* paramsBuffer, uint64_t offset);

        /// <summary>
        /// Transitions image layout
        /// </summary>
        /// <param name="image">Image handle</param>
        /// <param name="format">Image format</param>
        /// <param name="oldLayout">Previous layout</param>
        /// <param name="newLayout">New layout</param>
        /// <param name="range">Optional subresource range</param>
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                                   VkImageSubresourceRange* range = nullptr);

        /// <summary>
        /// Transitions texture layout using smart pointer
        /// </summary>
        /// <param name="texture">Smart pointer to texture</param>
        /// <param name="layout">New layout</param>
        /// <param name="range">Optional subresource range</param>
        void TransitionTextureLayout(const SPtr<Texture>& texture, VkImageLayout layout,
                                     VkImageSubresourceRange* range = nullptr);

        /// <summary>
        /// Transitions texture layout using raw pointer
        /// </summary>
        /// <param name="texture">Pointer to texture</param>
        /// <param name="layout">New layout</param>
        /// <param name="range">Optional subresource range</param>
        void TransitionTextureLayout(Texture* texture, VkImageLayout layout, VkImageSubresourceRange* range = nullptr);

        /// <summary>
        /// Copies data between buffers
        /// </summary>
        /// <param name="srcBuffer">Source buffer handle</param>
        /// <param name="dstBuffer">Destination buffer handle</param>
        /// <param name="size">Size of data to copy in bytes</param>
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        /// <summary>
        /// Copies buffer data to an image
        /// </summary>
        /// <param name="buffer">Source buffer</param>
        /// <param name="image">Destination image</param>
        /// <param name="width">Image width</param>
        /// <param name="height">Image height</param>
        /// <param name="regions">Optional copy regions</param>
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
                               std::vector<VkBufferImageCopy2>* regions = nullptr);

        /// <summary>
        /// Sets depth bias values
        /// </summary>
        /// <param name="slopeBias">Slope-based bias value</param>
        /// <param name="constantBias">Constant bias value</param>
        void SetDepthBias(float slopeBias, float constantBias);

        /// <summary>
        /// Binds a mesh for rendering
        /// </summary>
        /// <param name="mesh">Pointer to mesh object</param>
        void BindMesh(Mesh* mesh);

        /// <summary>
        /// Draws indexed geometry
        /// </summary>
        /// <param name="indexCount">Number of indices to draw</param>
        /// <param name="instanceCount">Number of instances</param>
        /// <param name="firstIndex">First index to draw</param>
        /// <param name="vertexOffset">Vertex buffer offset</param>
        /// <param name="firstInstance">First instance index</param>
        void DrawIndexed(uint32_t indexCount,
                         uint32_t instanceCount = 1,
                         uint32_t firstIndex = 0,
                         int32_t vertexOffset = 0,
                         uint32_t firstInstance = 0);

        /// <summary>
        /// Performs indirect drawing using parameters from a drawing buffer
        /// </summary>
        /// <param name="buffer">Buffer containing draw parameters</param>
        /// <param name="offset">Offset into buffer</param>
        /// <param name="drawCount">Number of draw commands</param>
        /// <param name="stride">Stride between draw commands</param>
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

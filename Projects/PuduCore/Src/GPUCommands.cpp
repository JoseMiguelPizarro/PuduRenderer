#include "GPUCommands.h"
#include "PuduGraphics.h"
#include "Resources/Resources.h"
#include "vulkan/vulkan.h"


#undef  VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vk_enum_string_helper.h"
#define VK_USE_PLATFORM_WIN32_KHR

#include <VulkanUtils.h>
#include <Logger.h>

namespace Pudu
{
    GPUCommands::GPUCommands(VkCommandBuffer handle, PuduGraphics* gfx) : vkHandle(handle), m_graphics(gfx)
    {
    }

    void GPUCommands::Clear(vec4 color)
    {
        m_clearValues[0] = {{color.x, color.y, color.z, color.w}};
    }

    void GPUCommands::ClearDepthStencil(float depth, float stencil)
    {
        m_clearValues[1] = {{depth, stencil}};
    }

    void GPUCommands::ImageBarrier(const VkImageMemoryBarrier2* barrier) const
    {
        VkDependencyInfo dependencyInfo{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = barrier;

        vkCmdPipelineBarrier2(vkHandle, &dependencyInfo);
    }

    void GPUCommands::AddImageBarrier(VkImage image, ResourceUsage oldState, ResourceUsage newState, u32 baseMipLevel,
                                      u32 mipCount, bool isDepth) const
    {
        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        VkDependencyInfo dependencyInfo{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = isDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = mipCount;

        barrier.subresourceRange.baseMipLevel = baseMipLevel;
        barrier.oldLayout = VkImageLayoutFromUsage(oldState);
        barrier.newLayout = VkImageLayoutFromUsage(newState);
        barrier.srcAccessMask = VkImageLayoutFromUsage(oldState);
        barrier.dstAccessMask = VkImageLayoutFromUsage(newState);

        const VkPipelineStageFlags sourceStageMask = DeterminePipelineStageFlags(
            barrier.srcAccessMask, QueueType::Graphics);
        const VkPipelineStageFlags destionationStageMask = DeterminePipelineStageFlags(
            barrier.dstAccessMask, QueueType::Graphics);

        //vkCmdPipelineBarrier2(vkHandle, &dependencyInfo);

        vkCmdPipelineBarrier(vkHandle, sourceStageMask, destionationStageMask, 0,
                             0, nullptr, 0, nullptr, 1, &barrier);
    }

    void GPUCommands::AddMemoryBarrier(VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask,
                                       VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask)
    {
        VkMemoryBarrier2 barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER_2};
        barrier.srcStageMask = srcStageMask;
        barrier.dstStageMask = dstStageMask;
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;

        VkDependencyInfo barrierInfo{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
        barrierInfo.memoryBarrierCount = 1;
        barrierInfo.pMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(vkHandle, &barrierInfo);
    }

    void GPUCommands::SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        VkRect2D scissor;

        if (width | height)
        {
            scissor.offset.x = x;
            scissor.offset.y = y;
            scissor.extent.width = width;
            scissor.extent.height = height;
        }
        else
        {
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            scissor.extent.width = 0;
            scissor.extent.height = 0;
        }

        vkCmdSetScissor(vkHandle, 0, 1, &scissor);
    }

    void GPUCommands::SetViewport(Viewport const& viewport)
    {
        VkViewport vkViewport;
        vkViewport.height = viewport.rect.height;
        vkViewport.width = viewport.rect.width;
        vkViewport.x = viewport.rect.x;
        vkViewport.y = viewport.rect.y;
        vkViewport.maxDepth = viewport.maxDepth;
        vkViewport.minDepth = viewport.minDepth;

        vkCmdSetViewport(vkHandle, 0, 1, &vkViewport);
    }

    void GPUCommands::PushConstants(VkPipelineLayout pipelineLayout, VkPipelineStageFlags stagesFlag, uint32_t offset,
                                    uint32_t size, void* data)
    {
        vkCmdPushConstants(vkHandle, pipelineLayout, stagesFlag, offset, size, data);
        m_hasRecordedCommand = true;
    }

    void GPUCommands::PushDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout pipelineLayout,
                                         uint32_t set, uint32_t writeCount, const VkWriteDescriptorSet* writes)
    {
        m_graphics->vkCmdPushDescriptorSetKHR(vkHandle, pipelineBindPoint, pipelineLayout, set, writeCount, writes);

        m_hasRecordedCommand = true;
    }

    void GPUCommands::UploadBufferData(GraphicsBuffer* buffer, const byte* data, const Size size, const Size offset) const
    {
        ASSERT(size % 4 == 0, "Buffer {} Size must be a multiple of 4", buffer->name);
        ASSERT(offset % 4 == 0, "Buffer {} Offset must be a multiple of 4", buffer->name);
        ASSERT(size <= 65536, "Buffer {} Size must be less or equal than 65536[bytes]", buffer->name);

        vkCmdUpdateBuffer(vkHandle, buffer->vkHandle, offset, size, data);
    }

    void GPUCommands::BufferBarrier(GraphicsBuffer* buffer, Size size, Size offset, VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask, u32 srcQueueIndex, u32 dstQueueIndex)
    {
        VkBufferMemoryBarrier2 bufferBarrier{};
        bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        bufferBarrier.srcAccessMask =  VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT; //Hardcoded for now
        bufferBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
        bufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        bufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        bufferBarrier.srcQueueFamilyIndex = srcQueueIndex;
        bufferBarrier.dstQueueFamilyIndex = dstQueueIndex;
        bufferBarrier.buffer = buffer->vkHandle;
        bufferBarrier.offset = offset;
        bufferBarrier.size = size;

        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext = nullptr;
        dependencyInfo.bufferMemoryBarrierCount = 1;
        dependencyInfo.pBufferMemoryBarriers =& bufferBarrier;
        vkCmdPipelineBarrier2(vkHandle,&dependencyInfo);
    }

    void GPUCommands::BegingRenderingPass(const VkRenderingInfo& renderInfo)
    {
        vkCmdBeginRendering(vkHandle, &renderInfo);
    }

    void GPUCommands::BegingRenderPass(const VkRenderPassBeginInfo& renderInfo)
    {
        vkCmdBeginRenderPass(vkHandle, &renderInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void GPUCommands::EndRenderPass()
    {
        vkCmdEndRenderPass(vkHandle);
    }

    //void GPUCommands::BindRenderPass(RenderPassHandle renderPassHandle, FramebufferHandle framebufferHandle)
    //{
    //	RenderPass* renderPass = m_graphics->Resources()->GetRenderPass(renderPassHandle);

    //	Framebuffer* framebuffer = m_graphics->Resources()->GetFramebuffer(framebufferHandle);

    //	if (renderPass != currentRenderPass)
    //	{
    //		VkRenderPassBeginInfo renderPassBegin{};
    //		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //		renderPassBegin.framebuffer = framebuffer->vkHandle;
    //		renderPassBegin.renderPass = renderPass->vkHandle;
    //		renderPassBegin.renderArea.offset = { 0,0 };
    //		renderPassBegin.renderArea.extent = { framebuffer->width,framebuffer->height };

    //		VkClearValue clearValues[K_MAX_IMAGE_OUTPUTS + 1]; //+1 for depthstencil
    //		uint32_t clearValuesCount = 0;
    //		for (uint32_t o = 0; o < renderPass->output.numColorFormats; ++o)
    //		{
    //			if (renderPass->output.colorOperations[o] == RenderPassOperation::Clear)
    //			{
    //				clearValues[clearValuesCount++] = m_clearValues[0];
    //			}
    //		}

    //		if (renderPass->output.depthStencilFormat != VK_FORMAT_UNDEFINED)
    //		{
    //			if (renderPass->output.depthOperation == RenderPassOperation::Clear)
    //			{
    //				clearValues[clearValuesCount++] = m_clearValues[1];
    //			}
    //		}

    //		renderPassBegin.clearValueCount = clearValuesCount;
    //		renderPassBegin.pClearValues = clearValues;

    //		vkCmdBeginRenderPass(vkHandle, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
    //	}

    //	currentRenderPass = renderPass;
    //	currentFramebuffer = framebuffer;
    //}

    void GPUCommands::EndRenderingPass()
    {
        vkCmdEndRendering(vkHandle);
    }

    void GPUCommands::BindPipeline(const Pipeline* pipeline)
    {
        vkCmdBindPipeline(vkHandle, pipeline->vkPipelineBindPoint, pipeline->vkHandle);

        // Cache pipeline

        m_hasRecordedCommand = true;
    }

    void GPUCommands::BindDescriptorSet(VkPipelineLayout pipelineLayout, VkDescriptorSet* handles,
                                        uint16_t handlesCount, uint32_t offset)
    {
        vkCmdBindDescriptorSets(vkHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, offset, handlesCount,
                                handles, 0, nullptr);
    }

    void GPUCommands::BindDescriptorSetCompute(VkPipelineLayout pipelineLayout, VkDescriptorSet* handles,
                                               uint16_t handlesCount)
    {
        vkCmdBindDescriptorSets(vkHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, handlesCount, handles, 0,
                                nullptr);

        m_hasRecordedCommand = true;
    }


    void GPUCommands::Blit(SPtr<Texture> source, SPtr<Texture> dst, VkImageLayout srcLayout, VkImageLayout dstLayout)
    {
        VkBlitImageInfo2 blitInfo{VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2};
        VkImageBlit2 blitRegion{VK_STRUCTURE_TYPE_IMAGE_BLIT_2};
        VkOffset3D offset[2] = {{0, 0, 0}, {0, 0, 0}};


        VkImageSubresourceLayers dstSubresource{};
        dstSubresource.aspectMask = TextureFormat::HasDepth(dst->format)
                                        ? VK_IMAGE_ASPECT_DEPTH_BIT
                                        : VK_IMAGE_ASPECT_COLOR_BIT;
        dstSubresource.mipLevel = 0;
        dstSubresource.layerCount = 1;
        dstSubresource.baseArrayLayer = 0;

        blitRegion.dstOffsets[0] = {0, 0, 0};
        blitRegion.dstOffsets[1] = {(int)dst->width, (int)dst->height, 1};
        blitRegion.srcOffsets[0] = {0, 0, 0};
        blitRegion.srcOffsets[1] = {(int)source->width, (int)source->height, 1};
        blitRegion.dstSubresource = dstSubresource;
        blitRegion.srcSubresource = dstSubresource;

        blitInfo.srcImage = source->vkImageHandle;
        blitInfo.srcImageLayout = srcLayout;
        blitInfo.dstImage = dst->vkImageHandle;
        blitInfo.dstImageLayout = dstLayout;

        blitInfo.pRegions = &blitRegion;
        blitInfo.regionCount = 1;

        vkCmdBlitImage2(vkHandle, &blitInfo);

        m_hasRecordedCommand = true;
    }

    void GPUCommands::Blit(SPtr<Texture> source, SPtr<Texture> dst, VkFilter filter, VkImageLayout srcLayout,
        VkImageLayout dstLayout, VkImageBlit2* regions, Size regionCount) const
    {
        Blit(source.get(), dst.get(), filter, srcLayout, dstLayout, regions, regionCount);
    }

    void GPUCommands::Blit(Texture* source, Texture* dst,VkFilter filter,  VkImageLayout srcLayout, VkImageLayout dstLayout, VkImageBlit2* regions, Size regionCount) const
    {
        VkBlitImageInfo2 blitInfo{VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2};
        blitInfo.srcImage = source->vkImageHandle;
        blitInfo.srcImageLayout = srcLayout;
        blitInfo.dstImage = dst->vkImageHandle;
        blitInfo.dstImageLayout = dstLayout;
        blitInfo.filter = filter;

        blitInfo.pRegions = regions;
        blitInfo.regionCount = regionCount;

        vkCmdBlitImage2(vkHandle, &blitInfo);
    }

    void GPUCommands::Dispatch(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        vkCmdDispatch(vkHandle, groupCountX, groupCountY, groupCountZ);

        m_hasRecordedCommand = true;
    }

    void GPUCommands::DispatchIndirect(GraphicsBuffer* paramsBuffer, uint64_t offset)
    {
        vkCmdDispatchIndirect(vkHandle, paramsBuffer->vkHandle, offset);
    }


    void GPUCommands::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                            VkImageLayout newLayout, VkImageSubresourceRange* range)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;

        if (range != nullptr)
        {
            barrier.subresourceRange = *range;
        }
        else
        {
            //Default range
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
        }

        barrier.image = image;
        barrier.subresourceRange.aspectMask = TextureFormat::HasDepth(format)
                                                  ? VK_IMAGE_ASPECT_DEPTH_BIT
                                                  : VK_IMAGE_ASPECT_COLOR_BIT;


        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || newLayout ==
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;

            if (TextureFormat::HasStencil(format))
            {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        if (TextureFormat::HasDepth(format))
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        VkPipelineStageFlags sourceStage{};
        VkPipelineStageFlags destinationStage{};

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout ==
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else if ((oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                oldLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)
            && (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR || newLayout == VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR ||
                newLayout == VK_IMAGE_LAYOUT_GENERAL))
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = 0;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
        else if ((oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL || oldLayout ==
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = 0;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = 0;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            barrier.dstAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

            //Top of pipe, we don't want to wait for operations to do the transition, that's why srcAccessMask = 0
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            //Unblock depth operations
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            //TODO: We need more information to know if we should block or not vertex and compute stages
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        }
        else if ((oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || oldLayout ==
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout ==
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        }else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout ==VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout ==VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            return;
        }
        else if (oldLayout ==VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout ==VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT; //TODO: JUST USE THE PROPER MASK, WE NEED MORE USAGE CONTEXT HERE
        }
        else
        {
            LOG_ERROR("unsupported layout transition! {}-> {}", string_VkImageLayout(oldLayout),
                      string_VkImageLayout(newLayout));
        }
        vkCmdPipelineBarrier(vkHandle, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        m_hasRecordedCommand = true;
    }

    void GPUCommands::TransitionTextureLayout(const SPtr<Texture>& texture, VkImageLayout layout,
        VkImageSubresourceRange* range)
    {
        TransitionTextureLayout(texture.get(), layout, range);
    }

    void GPUCommands::TransitionTextureLayout(Texture* texture, VkImageLayout layout, VkImageSubresourceRange* range)
    {
        TransitionImageLayout(texture->vkImageHandle,texture->format, texture->GetImageLayout(), layout,range);

        texture->SetImageLayout(layout);
    }

    void GPUCommands::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkBufferCopy copyRegion{};
        copyRegion.size = size;

        vkCmdCopyBuffer(vkHandle, srcBuffer, dstBuffer, 1, &copyRegion);

        m_hasRecordedCommand = true;
    }

    void GPUCommands::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
                                        std::vector<VkBufferImageCopy2>* regions)
    {
        uint32_t regionsCount = 1;

        VkBufferImageCopy2* data = nullptr;

        if (regions != nullptr && regions->size() > 0)
        {
            regionsCount = regions->size();
            data = regions->data();
        }
        else
        {
            VkBufferImageCopy2 region{VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = {0, 0, 0};
            region.imageExtent = {
                width,
                height,
                1
            };

            data = &region;
            regionsCount = 1;
        }

        VkCopyBufferToImageInfo2 copyInfo{VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2};
        copyInfo.srcBuffer = buffer;
        copyInfo.dstImage = image;
        copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copyInfo.pRegions = data;
        copyInfo.regionCount = regionsCount;

        vkCmdCopyBufferToImage2(
            vkHandle,
            &copyInfo
        );

        m_hasRecordedCommand = true;
    }

    void GPUCommands::SetDepthBias(float slopeBias, float constantBias)
    {
        vkCmdSetDepthBias(vkHandle, constantBias, 0.0f, slopeBias);
        m_hasRecordedCommand = true;
    }

    void GPUCommands::BindMesh(Mesh* mesh)
    {
        VkBuffer vertexBuffers[] = {mesh->GetVertexBuffer()->vkHandle};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(vkHandle, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(vkHandle, mesh->GetIndexBuffer()->vkHandle, 0, VK_INDEX_TYPE_UINT32);

        m_hasRecordedCommand = true;
    }

    void GPUCommands::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                  int32_t vertexOffset, uint32_t firstInstance)
    {
        vkCmdDrawIndexed(vkHandle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

        m_hasRecordedCommand = true;
    }

    void GPUCommands::DrawIndirect(GraphicsBuffer* buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
    {
        vkCmdDrawIndirect(vkHandle, buffer->vkHandle, offset, drawCount, stride);
        m_hasRecordedCommand = true;
    }

    void GPUCommands::Reset()
    {
        m_hasRecordedCommand = false;
        vkResetCommandBuffer(vkHandle, 0);
    }

    void GPUCommands::BeginCommands()
    {
        m_hasRecordedCommand = true;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(vkHandle, &beginInfo) != VK_SUCCESS)
        {
            LOG_ERROR("failed to begin recording command buffer!");
        }
    }

    void GPUCommands::EndCommands()
    {
        vkEndCommandBuffer(vkHandle);
    }
}

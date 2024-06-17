#include "GPUCommands.h"
#include "PuduGraphics.h"
#include "Resources/Resources.h"
#include "vulkan/vulkan.h"
#include <VulkanUtils.h>
#include <Logger.h>

namespace Pudu
{
	GPUCommands::GPUCommands(VkCommandBuffer handle, PuduGraphics* gfx) : vkHandle(handle), m_graphics(gfx)
	{
	}

	void GPUCommands::Clear(vec4 color)
	{
		m_clearValues[0] = { {color.x,color.y,color.z,color.w} };
	}

	void GPUCommands::ClearDepthStencil(float depth, float stencil)
	{
		m_clearValues[1] = { {depth,stencil} };
	}
	void GPUCommands::AddImageBarrier(VkImage image, ResourceState oldState, ResourceState newState, u32 baseMipLevel, u32 mipCount, bool isDepth)
	{
		VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		VkDependencyInfo dependencyInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
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

		const VkPipelineStageFlags sourceStageMask = DeterminePipelineStageFlags(barrier.srcAccessMask, QueueType::Graphics);
		const VkPipelineStageFlags destionationStageMask = DeterminePipelineStageFlags(barrier.dstAccessMask, QueueType::Graphics);

		//vkCmdPipelineBarrier2(vkHandle, &dependencyInfo);

		vkCmdPipelineBarrier(vkHandle, sourceStageMask, destionationStageMask, 0,
			0, nullptr, 0, nullptr, 1, &barrier);
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
	void GPUCommands::BindRenderPass(RenderPassHandle renderPassHandle, FramebufferHandle framebufferHandle)
	{
		RenderPass* renderPass = m_graphics->Resources()->GetRenderPass(renderPassHandle);

		Framebuffer* framebuffer = m_graphics->Resources()->GetFramebuffer(framebufferHandle);

		if (renderPass != currentRenderPass)
		{
			VkRenderPassBeginInfo renderPassBegin{};
			renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBegin.framebuffer = framebuffer->vkHandle;
			renderPassBegin.renderPass = renderPass->vkHandle;
			renderPassBegin.renderArea.offset = { 0,0 };
			renderPassBegin.renderArea.extent = { framebuffer->width,framebuffer->height };

			VkClearValue clearValues[K_MAX_IMAGE_OUTPUTS + 1]; //+1 for depthstencil
			uint32_t clearValuesCount = 0;
			for (uint32_t o = 0; o < renderPass->output.numColorFormats; ++o)
			{
				if (renderPass->output.colorOperations[o] == RenderPassOperation::Clear)
				{
					clearValues[clearValuesCount++] = m_clearValues[0];
				}
			}

			if (renderPass->output.depthStencilFormat != VK_FORMAT_UNDEFINED)
			{
				if (renderPass->output.depthOperation == RenderPassOperation::Clear)
				{
					clearValues[clearValuesCount++] = m_clearValues[1];
				}
			}

			renderPassBegin.clearValueCount = clearValuesCount;
			renderPassBegin.pClearValues = clearValues;

			vkCmdBeginRenderPass(vkHandle, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
		}

		currentRenderPass = renderPass;
		currentFramebuffer = framebuffer;
	}

	void GPUCommands::EndCurrentRenderPass()
	{
		if (currentRenderPass)
		{
			vkCmdEndRenderPass(vkHandle);
		}

		currentRenderPass = nullptr;
	}
	void GPUCommands::BindPipeline(Pipeline* pipeline)
	{
		vkCmdBindPipeline(vkHandle, pipeline->vkPipelineBindPoint, pipeline->vkHandle);

		// Cache pipeline
		currentPipeline = pipeline;

		m_hasRecordedCommand = true;
	}

	void GPUCommands::BindDescriptorSet(VkDescriptorSet handle)
	{
		//TODO: IMPLEMENT
	}

	void GPUCommands::BindDescriptorSetCompute(VkPipelineLayout pipelineLayout, VkDescriptorSet* handles, uint16_t handlesCount)
	{
		vkCmdBindDescriptorSets(vkHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, handlesCount, handles, 0, nullptr);

		m_hasRecordedCommand = true;
	}


	void GPUCommands::Blit(SPtr<Texture2d> source, SPtr<Texture2d> dst, VkImageLayout srcLayout, VkImageLayout dstLayout)
	{
		VkBlitImageInfo2 blitInfo{ VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2 };
		VkImageBlit2 blitRegion{ VK_STRUCTURE_TYPE_IMAGE_BLIT_2 };
		VkOffset3D offset[2] = { {0,0,0},{0,0,0} };

		VkImageSubresourceLayers dstSubresource{};
		dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		dstSubresource.mipLevel = 0;
		dstSubresource.layerCount = 1;
		dstSubresource.baseArrayLayer = 0;

		blitRegion.dstOffsets[0] = { 0,0,0 };
		blitRegion.dstOffsets[1] = { (int)dst->width,(int)dst->height,1 };
		blitRegion.srcOffsets[0] = { 0,0,0 };
		blitRegion.srcOffsets[1] = { (int)source->width,(int)source->height,1 };
		blitRegion.dstSubresource = dstSubresource;
		blitRegion.srcSubresource = dstSubresource;

		blitInfo.srcImage = source->vkImageHandle;
		blitInfo.srcImageLayout = srcLayout;
		blitInfo.dstImage = dst->vkImageHandle;
		blitInfo.dstImageLayout = dstLayout;

		blitInfo.pRegions = &blitRegion;
		blitInfo.regionCount = 1;

		vkCmdBlitImage2(vkHandle, &blitInfo);
	}
	void GPUCommands::Dispatch(uint groupCountX, uint groupCountY, uint groupCountZ)
	{
		vkCmdDispatch(vkHandle, groupCountX, groupCountY, groupCountZ);

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
			PUDU_ERROR("failed to begin recording command buffer!");
		}
	}
	void GPUCommands::EndCommands()
	{
		vkEndCommandBuffer(vkHandle);
	}
}

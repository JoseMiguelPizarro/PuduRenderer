#include "GPUCommands.h"
#include "PuduGraphics.h"
#include "Resources/Resources.h"

#include <VulkanUtils.h>

namespace Pudu
{
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
	}
	void GPUCommands::BindRenderPass(RenderPassHandle renderPassHandle, FramebufferHandle framebufferHandle)
	{
		RenderPass* renderPass = graphics->Resources()->GetRenderPass(renderPassHandle);

		Framebuffer* framebuffer = graphics->Resources()->GetFramebuffer(framebufferHandle);

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
	}

	void GPUCommands::Blit(SPtr<Texture2d> source, SPtr<Texture2d> dst, VkImageLayout dstLayout)
	{
		VkBlitImageInfo2 blitInfo{};
		blitInfo.srcImage = source->vkImageHandle;
		blitInfo.srcImageLayout = source->vkImageLayout;
		blitInfo.dstImage = dst->vkImageHandle;
		blitInfo.dstImageLayout = dstLayout;
	}
}

#pragma once
#include <vulkan/vulkan_core.h>
#include "Resources/Resources.h"
#include "GPUEnums.h"

namespace Pudu {

    static VkCullModeFlagBits ToVkCullMode(CullMode cullMode) {
        switch (cullMode)
        {
        case Pudu::Front:
            return VK_CULL_MODE_FRONT_BIT;
            break;
        case Pudu::Back:
            return VK_CULL_MODE_BACK_BIT;
            break;
        case Pudu::Both:
            return VK_CULL_MODE_FRONT_AND_BACK;
            break;
        case Pudu::None:
            return VK_CULL_MODE_NONE;
            break;
        default:
            return VK_CULL_MODE_BACK_BIT;
            break;
        }
    }

	static VkImageLayout VkImageLayoutFromUsage(ResourceState usage) {
		if (usage & RESOURCE_STATE_COPY_SOURCE)
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		if (usage & RESOURCE_STATE_COPY_DEST)
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		if (usage & RESOURCE_STATE_RENDER_TARGET)
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		if (usage & RESOURCE_STATE_DEPTH_WRITE)
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		if (usage & RESOURCE_STATE_DEPTH_READ)
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		if (usage & RESOURCE_STATE_UNORDERED_ACCESS)
			return VK_IMAGE_LAYOUT_GENERAL;

		if (usage & (RESOURCE_STATE_SHADER_RESOURCE| RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (usage & RESOURCE_STATE_PRESENT)
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		if (usage == RESOURCE_STATE_COMMON)
			return VK_IMAGE_LAYOUT_GENERAL;

		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

    // Determines pipeline stages involved for given accesses
    static VkPipelineStageFlags DeterminePipelineStageFlags(VkAccessFlags accessFlags, QueueType::Enum queueType) {
        VkPipelineStageFlags flags = 0;

        switch (queueType) {
        case QueueType::Graphics:
        {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0) {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }

            if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            if ((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

            break;
        }
        case QueueType::Compute:
        {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0 ||
                (accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0 ||
                (accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0 ||
                (accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

            break;
        }
        case QueueType::CopyTransfer: return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        default: break;
        }

        // Compatible with both compute and graphics queues
        if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
            flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

        if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
            flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

        if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
            flags |= VK_PIPELINE_STAGE_HOST_BIT;

        if (flags == 0)
            flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        return flags;
    }


#ifndef VKCheck
#define VKCheck(result, message) if(result != VK_SUCCESS)\
    {\
    PUDU_ERROR(message);\
    }
#endif // !VKCheck

}

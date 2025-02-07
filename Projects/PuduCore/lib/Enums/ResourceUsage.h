//
// Created by Hojaverde on 2/1/2025.
//

#pragma once

namespace Pudu
{
    enum ResourceUsage
    {
            UNDEFINED = 0,
            VERTEX_AND_CONSTANT_BUFFER = 0x1,
            INDEX_BUFFER = 0x2,
            RENDER_TARGET = 0x4,
            UNORDERED_ACCESS = 0x8,
            DEPTH_WRITE = 0x10,
            DEPTH_READ = 0x20,
            NON_PIXEL_SHADER_RESOURCE = 0x40,
            PIXEL_SHADER_RESOURCE = 0x80,
            SHADER_RESOURCE = 0x40 | 0x80,
            STREAM_OUT = 0x100,
            INDIRECT_ARGUMENT = 0x200,
            COPY_DEST = 0x400,
            COPY_SOURCE = 0x800,
            PRESENT = 0x1000,
            COMMON = 0x2000,
    };

    static VkImageLayout VkImageLayoutFromUsage(const ResourceUsage usage) {
        if (usage & COPY_SOURCE)
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        if (usage & COPY_DEST)
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        if (usage & RENDER_TARGET)
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (usage & DEPTH_WRITE)
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        if (usage & DEPTH_READ)
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        if (usage & UNORDERED_ACCESS)
            return VK_IMAGE_LAYOUT_GENERAL;

        if (usage & (SHADER_RESOURCE| PIXEL_SHADER_RESOURCE))
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        if (usage & PRESENT)
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        if (usage == COMMON)
            return VK_IMAGE_LAYOUT_GENERAL;

        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

//
// Created by Hojaverde on 8/13/2025.
//

#pragma once

#include "PuduCore.h"
#include "GPUEnums.h"

namespace Pudu
{
    ENUM_STRING(VertexAttributeType,
                UNDEFINED,
                POSITION,
                NORMAL,
                TANGENT,
                COLOR,
                TEXCOORD0,
                TEXCOORD1,
                TEXCOORD2,
                TEXCOORD3,
    );

    ENUM_STRING(VertexInputRate,
                PerVertex,
                PerInstance);

    struct VertexAttribute
    {
        VertexAttributeType type;
        ChannelFormat format;

        VertexAttribute() = default;
        VertexAttribute(VertexAttributeType type, ChannelFormat format);
        Size GetStride() {return GetChannelFormatSize(format);}

    private:
        friend struct VertexLayout;
        friend class PuduGraphics;
        u32 location;
        u32 offset;
        u32 binding = 0; //TODO: SUPPORT DYNAMIC BINDING
    };

    // Rate at which an attribute is pulled from the buffer
    struct VertexStream
    {
        u16 binding = 0;
        u16 stride = 0;
        VertexInputRate inputRate = VertexInputRate::PerInstance;

        VkVertexInputRate GetVkInputRate() const
        {
            return inputRate == VertexInputRate::PerVertex
                       ? VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX
                       : VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE;
        }
    };

    struct VertexLayout
    {
        VertexLayout() = default;
        VertexLayout& PushAttribute(VertexAttribute attribute);
        u32 GetAttributeCount() const { return m_attributeCount; }
        VertexAttribute GetAttribute(u32 attributeIndex) const { return m_attributes[attributeIndex]; }
        Size GetStride() const { return m_stride; }

    private:
        VertexAttribute m_attributes[K_MAX_VERTEX_ATTRIBUTES];
        u32 m_attributeCount = 0;
        Size m_stride = 0;
    };

    struct VertexInputCreation
    {
        VertexInputCreation() = default;
        VertexInputCreation(VertexLayout* layout);
        VertexLayout* GetVertexLayout();
        VertexInputCreation& PushVertexStream(VertexStream stream);
        VertexStream GetVertexStream(u32 streamIndex) const { return m_streams[streamIndex]; }
        u32 GetStreamCount() const { return m_vertexStreamCount; }

    private:
        VertexLayout* m_vertexLayout;
        VertexStream m_streams[K_MAX_VERTEX_STREAMS];
        u32 m_vertexStreamCount = 0;

    };
}

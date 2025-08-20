//
// Created by Administrator on 8/13/2025.
//

#include "VertexLayout.h"
#include "Logger.h"

namespace Pudu
{
    VertexAttribute::VertexAttribute(VertexAttributeType type, ChannelFormat format): location(0), offset(0)
    {
        this->type = type;
        this->format = format;
    }

    VertexLayout& VertexLayout::PushAttribute(VertexAttribute attribute)
    {
        ASSERT(m_attributeCount < K_MAX_VERTEX_ATTRIBUTES, "Reached max vertex attributes [{}]",
               K_MAX_VERTEX_ATTRIBUTES);

        auto formatSize = GetChannelFormatSize(attribute.format);
        attribute.offset = 0;
        attribute.location = m_attributeCount;
        attribute.binding = m_attributeCount; //Binding to per-attribute vertex stream

        m_attributes[m_attributeCount] = attribute;

        m_stride += formatSize;
        m_attributeCount++;
        return *this;
    }

    VertexInputCreation::VertexInputCreation(VertexLayout* layout)
    {
        m_vertexLayout = layout;
    }

    VertexLayout* VertexInputCreation::GetVertexLayout()
    {
        return m_vertexLayout;
    }

    VertexInputCreation& VertexInputCreation::PushVertexStream(VertexStream stream)
    {
        ASSERT(m_vertexStreamCount < K_MAX_VERTEX_STREAMS, "Reached max vertex streams [{}]", K_MAX_VERTEX_STREAMS);
        ASSERT(stream.stride > 0, "Trying to create a vertex stream with a stride of 0");

        m_streams[m_vertexStreamCount++] = stream;

        return *this;
    }
}

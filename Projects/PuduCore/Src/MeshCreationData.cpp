#include "MeshCreationData.h"

#include "Logger.h"


namespace Pudu
{
    VertexAttributeStream& MeshAttributes::CreateAttribute(const VertexAttribute& attribute, Size count)
    {
        ASSERT(m_attributeCount < K_MAX_VERTEX_ATTRIBUTES, "Reached max vertex attributes [{} won't be set].", ToString(attribute.type));

        for (auto& stream : m_vertexAttributeStream)
        {
            if (stream.Attribute.type == attribute.type)
            {
                LOG_ERROR("Trying to create attribute {} twice", ToString(attribute.type));
            }
        }

        VertexAttributeStream& stream = m_vertexAttributeStream[m_attributeCount++];
        stream.Attribute = attribute;
        stream.Count = count;
        stream.Stride = GetChannelFormatSize(attribute.format);

        stream.Data = new byte[count * GetChannelFormatSize(attribute.format)];

        return stream;
    }

    void MeshAttributes::PushVertexAttributeStream(VertexAttributeStream& stream)
    {
        ASSERT(m_attributeCount < K_MAX_VERTEX_ATTRIBUTES, "Reached max vertex attributes [{} won't be set].", ToString(stream.Attribute.type));

        m_vertexAttributeStream[m_attributeCount++] = stream;
    }

    void MeshAttributes::SetAttributeData(VertexAttributeType attribute, const void* data)
    {
        for (auto& stream : m_vertexAttributeStream)
        {
            if (stream.Attribute.type == attribute)
            {
                memcpy(stream.Data, data, stream.Count * stream.Stride);
                return;
            }
        }

        ASSERT(false, "Trying to set attribute data for non existing attribute {}", ToString(attribute));
    }

    SPtr<MeshAttributes> MeshAttributes::Create()
    {
        return std::make_shared<MeshAttributes>();
    }

    std::optional<VertexAttributeStream*> MeshAttributes::FindVertexAttributeStream(VertexAttributeType attribute)
    {
        for (auto& stream : m_vertexAttributeStream)
        {
            if (stream.Attribute.type == attribute)
            {
                return &stream;
            }
        }

        return std::nullopt;
    }

    void MeshAttributes::Destroy()
    {
        if (!m_disposed)
        {
            for (Size i = 0; i < m_attributeCount; i++)
            {
                auto stream = m_vertexAttributeStream[i];
                delete[] stream.Data;
            }

            m_disposed = true;
        }
    }

    MeshAttributes::~MeshAttributes()
    {
        Destroy();
    }
}

#pragma once
#include <vector>

#include "Logger.h"
#include "VertexLayout.h"
#include "MaterialCreationData.h"

namespace Pudu
{
    struct VertexAttributeStream
    {
        VertexAttribute Attribute;
        Size Count;
        Size Stride;
        byte* Data;
        VertexAttributeStream() = default;

        template<typename T>
        T* GetData()
        {
            return reinterpret_cast<T*>(Data);
        }

        std::string GetName();
    };

    struct MeshAttributes
    {
        MeshAttributes() = default;
        VertexAttributeStream& CreateAttribute(const VertexAttribute& attribute, Size count);
        void PushVertexAttributeStream(VertexAttributeStream& stream);
        void SetAttributeData(VertexAttributeType attribute, const void* data);

        static SPtr<MeshAttributes> Create();
        Size GetAttributeCount() const {return m_attributeCount;};
        std::optional<VertexAttributeStream*> FindVertexAttributeStream(VertexAttributeType attribute);

        VertexAttributeStream& GetAttributeStream(Size index)
        {
            ASSERT(index < m_attributeCount, "Trying to get attribute stream with index out of bounds {}", index);
            return m_vertexAttributeStream[index];
        };

        void Destroy();
        ~MeshAttributes();

    private:
        VertexAttributeStream m_vertexAttributeStream [K_MAX_VERTEX_ATTRIBUTES];
        bool m_disposed = false;
        Size m_attributeCount = 0;
    };

    struct MeshCreationData
    {
        struct ModelLayout
        {
            u32 StartVertexIndex;
            u32 VertexCount;

            u32 StartIndiceIndex;
            u32 IndicesCount;
        };

        std::string Name;

        std::vector<u32> Indices;
        std::vector<ModelLayout> ModelLayouts;
        bool HasTangents;
        MaterialCreationData Material;
        SPtr<MeshAttributes> MeshAttributes;
    };

}

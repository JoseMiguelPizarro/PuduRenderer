#pragma once
#include <vector>

#include "VertexLayout.h"
#include "MaterialCreationData.h"

namespace Pudu
{
    struct VertexAttributeStream
    {
        VertexAttribute Attribute;
        Size Count;
        Size Stride;
        void* Data;
        VertexAttributeStream() = default;

        template<typename T>
        T* GetData()
        {
            return static_cast<T*>(Data);
        }
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

        std::vector<VertexAttributeStream> VertexAttributeStreams;
        std::vector<u32> Indices;
        std::vector<ModelLayout> ModelLayouts;
        bool HasTangents;
        MaterialCreationData Material;
    };
}

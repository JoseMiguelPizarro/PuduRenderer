﻿#pragma once
#include <vector>

#include "vertex.h"
#include "Material.h"
#include "MaterialCreationData.h"

namespace Pudu
{
    struct MeshCreationData
    {
        struct ModelLayout
        {
            uint32_t StartVertexIndex;
            uint32_t VertexCount;

            uint32_t StartIndiceIndex;
            uint32_t IndicesCount;
        };

        std::string Name;
        std::vector<Vertex> Vertices;
        std::vector<uint32_t> Indices;
        std::vector<ModelLayout> ModelLayouts;
        MaterialCreationData Material;
    };
}

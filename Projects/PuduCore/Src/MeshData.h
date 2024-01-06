#pragma once
#include <vector>

#include "vertex.h"


namespace Pudu
{
    struct MeshData
    {
    public:
        std::vector<Vertex> Vertices;
        std::vector<uint32_t> Indices;
    };
}

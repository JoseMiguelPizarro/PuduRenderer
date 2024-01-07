#include "Mesh.h"

namespace Pudu
{
    GraphicsBuffer* Mesh::GetVertexBuffer()
    {
        return &m_vertexBuffer;
    }

    GraphicsBuffer* Mesh::GetIndexBuffer()
    {
        return &m_indexBuffer;
    }

    std::vector<Vertex>* Mesh::GetVertices()
    {
        return &m_vertices;
    }

    std::vector<uint32_t>* Mesh::GetIndices()
    {
        return &m_indices;
    }
}
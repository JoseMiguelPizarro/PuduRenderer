#pragma once
#include <vector>

#include "GraphicsBuffer.h"
#include "vertex.h"

namespace Pudu
{
    class Mesh
    {
    public:
        GraphicsBuffer* GetVertexBuffer();
        GraphicsBuffer* GetIndexBuffer();

        std::vector<Vertex>* GetVertices();
        std::vector<uint32_t>* GetIndices();

        Mesh() = default;

        void SetIndexBuffer(GraphicsBuffer buffer)
        {
            m_indexBuffer = buffer;
        }

        void SetVertexBuffer(GraphicsBuffer buffer)
        {
            m_vertexBuffer = buffer;
        }

        Mesh(
            GraphicsBuffer vertexBuffer,
            GraphicsBuffer indexBuffer,
            std::vector<Vertex> vertices,
            std::vector<uint32_t> indices):
            m_vertexBuffer(vertexBuffer), m_indexBuffer(indexBuffer),
            m_vertices(std::move(vertices)), m_indices(std::move(indices))
        {
        }

    private:
        GraphicsBuffer m_vertexBuffer;
        GraphicsBuffer m_indexBuffer;
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
    };
}

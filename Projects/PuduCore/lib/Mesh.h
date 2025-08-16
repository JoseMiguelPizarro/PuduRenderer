#pragma once
#include <vector>

#include "GraphicsBuffer.h"
#include "MeshCreationData.h"
#include "vertex.h"
#include "Resources/Resources.h"

namespace Pudu
{
    class Mesh : public GPUResource<Mesh>
    {
    public:
        std::vector<SPtr<GraphicsBuffer>> GetAttributeStreamBuffers();
        SPtr<GraphicsBuffer> GetIndexBuffer();

        std::vector<VertexAttributeStream>& GetVertexAttributeStreams();
        std::vector<u32>* GetIndices();

        Mesh() = default;
        ~Mesh();

        void SetIndexBuffer(SPtr<GraphicsBuffer> buffer)
        {
            m_indexBuffer = buffer;
        }

        void SetVertexAttributeBuffers(std::vector<SPtr<GraphicsBuffer>> buffers)
        {
            m_vertexAttributeStreamBuffers = buffers;
        }

        void SetVertexAttributeStreams(std::vector<VertexAttributeStream> streams)
        {
            m_vertexAttributeStreams = streams;
        }

        bool IsDisposed();

        std::string* GetName();

    private:
        friend PuduGraphics;

        void Destroy();

    public:
        GPUResourceType Type() override { return GPUResourceType::Mesh; }

    private:
        std::vector<SPtr<GraphicsBuffer>> m_vertexAttributeStreamBuffers;
        std::vector<VertexAttributeStream> m_vertexAttributeStreams;
        SPtr<GraphicsBuffer> m_indexBuffer;
        std::vector<uint32_t> m_indices;
        bool m_disposed;
    };
}

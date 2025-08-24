#pragma once
#include <vector>

#include "GraphicsBuffer.h"
#include "MeshCreationData.h"

namespace Pudu
{
    class Mesh : public GPUResource<Mesh>
    {
    public:
        std::vector<SPtr<GraphicsBuffer>> GetAttributeStreamBuffers();
        SPtr<GraphicsBuffer> GetIndexBuffer();

        MeshAttributes* GetVertexAttributeStreams() const;
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

        bool IsDisposed();

        virtual void OnCreate(PuduGraphics* gpu) override;

        std::string* GetName();
        bool HasTangents();

    private:
        friend PuduGraphics;

        void Destroy();

    public:
        GPUResourceType Type() override { return GPUResourceType::Mesh; }

    private:
        std::vector<SPtr<GraphicsBuffer>> m_vertexAttributeStreamBuffers;
        SPtr<MeshAttributes> m_attributes;
        SPtr<GraphicsBuffer> m_indexBuffer;
        std::vector<uint32_t> m_indices;
        bool m_disposed;
        bool m_hasTangents;
    };
}

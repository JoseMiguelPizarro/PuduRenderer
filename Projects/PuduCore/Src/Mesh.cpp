#include "PuduGraphics.h"
#include "Mesh.h"

namespace Pudu
{
	std::vector<SPtr<GraphicsBuffer>> Mesh::GetAttributeStreamBuffers()
	{
		return m_vertexAttributeStreamBuffers;
	}

	SPtr<GraphicsBuffer> Mesh::GetIndexBuffer()
	{
		return m_indexBuffer;
	}

	MeshAttributes* Mesh::GetVertexAttributeStreams() const
	{
		return m_attributes.get();
	}

	std::vector<uint32_t>* Mesh::GetIndices()
	{
		return &m_indices;
	}

	Mesh::~Mesh()
	{
		Destroy();
	}

	void Mesh::Destroy()
	{
		if (!m_disposed)
		{
			m_disposed = true;
			m_attributes->Destroy();
		}
		else
		{
			LOG("Disposing Mesh already disposed");
		}
	}
	bool Mesh::IsDisposed()
	{
		return m_disposed;
	}

	void Mesh::OnCreate(PuduGraphics* gpu)
	{
		gpu->AllocateMeshGPUData(this);
	}

	std::string* Mesh::GetName()
	{
		return &name;
	}

	bool Mesh::HasTangents()
	{
		return m_hasTangents;
	}
}

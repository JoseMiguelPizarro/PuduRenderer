#include "PuduGraphics.h"
#include "Mesh.h"

namespace Pudu
{
	SPtr<GraphicsBuffer> Mesh::GetVertexBuffer()
	{
		return m_vertexBuffer;
	}

	SPtr<GraphicsBuffer> Mesh::GetIndexBuffer()
	{
		return m_indexBuffer;
	}

	std::vector<Vertex>* Mesh::GetVertices()
	{
		return &m_vertices;
	}

	std::vector<uint32_t>* Mesh::GetIndices()
	{
		return &m_indices;
	}

	void Mesh::Destroy()
	{
		if (!m_disposed)
		{
			m_disposed = true;
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
	std::string* Mesh::GetName()
	{
		return &m_name;
	}
}
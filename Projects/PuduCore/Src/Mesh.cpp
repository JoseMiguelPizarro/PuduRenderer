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

	std::vector<VertexAttributeStream>& Mesh::GetVertexAttributeStreams()
	{
		return m_vertexAttributeStreams;
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
			for (auto& stream : m_vertexAttributeStreams)
			{
				delete stream.Data;
			}
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
		return &name;
	}
}
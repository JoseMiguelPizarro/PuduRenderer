#pragma once
#include <vector>

#include "GraphicsBuffer.h"
#include "vertex.h"
#include "Resources/Resources.h"

namespace Pudu
{
	class Mesh :public GPUResource<Mesh>
	{
	public:
		SPtr<GraphicsBuffer> GetVertexBuffer();
		SPtr<GraphicsBuffer> GetIndexBuffer();

		std::vector<Vertex>* GetVertices();
		std::vector<uint32_t>* GetIndices();

		Mesh() = default;

		void SetIndexBuffer(SPtr<GraphicsBuffer> buffer)
		{
			m_indexBuffer = buffer;
		}

		void SetVertexBuffer(SPtr<GraphicsBuffer> buffer)
		{
			m_vertexBuffer = buffer;
		}

		bool IsDisposed();

		std::string* GetName();

	private:
		friend PuduGraphics;

		void Destroy();
		SPtr<GraphicsBuffer> m_vertexBuffer;
		SPtr<GraphicsBuffer> m_indexBuffer;
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;
		bool m_disposed;
		std::string m_name;
	};
}

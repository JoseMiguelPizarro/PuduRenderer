#include "GraphicsBuffer.h"

namespace Pudu
{
	bool GraphicsBuffer::IsDestroyed()
	{
		return m_disposed;
	}
	void GraphicsBuffer::Destroy()
	{
		m_disposed = true;
	}
}

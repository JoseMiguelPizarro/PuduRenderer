#include "GraphicsBuffer.h"
#include "vma/vk_mem_alloc.h"

namespace Pudu
{
	uint64_t GraphicsBuffer::GetOffset()
	{
		return allocationInfo.offset;
	}
	uint64_t GraphicsBuffer::GetSize()
	{
		return allocationInfo.size;
	}

	void* GraphicsBuffer::GetMappedData()
	{
		return allocationInfo.pMappedData;
	}

	bool GraphicsBuffer::IsDestroyed()
	{
		return m_disposed;
	}
	void GraphicsBuffer::Destroy()
	{
		m_disposed = true;
	}
}

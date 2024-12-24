#include "MemoryAllocation.h"

namespace Pudu {

	uint64_t MemoryAllocation::GetOffset()
	{
		return m_offset;
	}
	uint64_t MemoryAllocation::GetSize()
	{
		return m_size;
	}
	void* MemoryAllocation::GetMappedData()
	{
		return m_mappedData;
	}
}

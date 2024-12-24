#pragma once
#include <cstdint>

namespace Pudu {
	struct MemoryAllocation
	{
		uint64_t GetOffset();
		uint64_t GetSize();
		void* GetMappedData();


	private:
		friend class PuduGraphics;
		uint64_t m_offset;
		uint64_t m_size;
		void* m_mappedData;
	};
}


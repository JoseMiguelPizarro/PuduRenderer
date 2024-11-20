#pragma once
#include <concepts>

namespace Pudu
{
	struct GPUResourceHandle
	{
		uint32_t index;
	};

	class GPUResource
	{
	public:
		GPUResourceHandle Handle() { return m_handle; }

	private:
		friend class GPUResourcesManager;
		GPUResourceHandle m_handle;


	};
}
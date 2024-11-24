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
		bool IsAllocated() { return m_allocated; }


	private:
		friend class GPUResourcesManager;
		GPUResourceHandle m_handle;
		bool m_allocated;
	};


	struct TextureHandle :public GPUResourceHandle
	{
	}; // struct TextureHandle
}
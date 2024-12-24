#pragma once
#include <vulkan/vulkan.h>
#include "Resources/GPUResource.h"
#include "Resources/Resources.h"
#include "vma/vk_mem_alloc.h"

namespace Pudu
{
	class GraphicsBuffer :public GPUResource<GraphicsBuffer>
	{
	public:
		VkBuffer vkHandle;
		GPUResourceHandle<GraphicsBuffer> handle;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
		std::string name;

		uint64_t GetOffset();
		uint64_t GetSize();
		void* GetMappedData();
		GraphicsBuffer() = default;

		bool IsDestroyed();

	private:
		friend PuduGraphics;
		void Destroy();
		bool m_disposed;
	};
};

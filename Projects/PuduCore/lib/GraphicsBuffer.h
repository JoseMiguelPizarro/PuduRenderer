#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "Resources/GPUResource.h"
#include "Resources/Resources.h"

namespace Pudu
{
	class GraphicsBuffer :public GPUResource<GraphicsBuffer>
	{
	public:
		VkBuffer vkHandler;
		GPUResourceHandle<GraphicsBuffer> handle;
		VmaAllocation allocation;
		std::string name;

		GraphicsBuffer() = default;

		bool IsDestroyed();

	private:
		friend PuduGraphics;
		void Destroy();
		bool m_disposed;
	};
};

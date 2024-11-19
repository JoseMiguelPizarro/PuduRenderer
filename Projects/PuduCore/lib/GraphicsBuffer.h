#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "Resources/Resources.h"

namespace Pudu
{
	class GraphicsBuffer
	{
	public:
		VkBuffer vkHandler;
		GraphicsBufferHandle handle;
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

#pragma once
#include <vulkan/vulkan.h>
#include "Resources/Resources.h"


namespace Pudu
{
	class GraphicsBuffer
	{
	public:
		VkBuffer vkHandler;
		VkDeviceMemory DeviceMemoryHandler;
		GraphicsBufferHandle handle;
		void* MappedMemory = nullptr;

		GraphicsBuffer() = default;

		bool IsDestroyed();

	private:
		friend PuduGraphics;
		void Destroy();
		bool m_disposed;
	};
};

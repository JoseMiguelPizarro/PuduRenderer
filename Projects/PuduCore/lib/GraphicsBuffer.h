#pragma once
#include <vulkan/vulkan.h>
class GraphicsBuffer
{
public:
	VkBuffer  Buffer;
	VkDeviceMemory DeviceMemory;

	GraphicsBuffer() = default;

	GraphicsBuffer(VkBuffer buffer, VkDeviceMemory deviceMemory)
		: Buffer(buffer), DeviceMemory(deviceMemory)
	{

	}
};


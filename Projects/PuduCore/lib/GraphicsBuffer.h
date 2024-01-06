#pragma once
#include <vulkan/vulkan.h>

class GraphicsBuffer
{
public:
    VkBuffer Handler;
    VkDeviceMemory DeviceMemoryHandler;
    void* MappedMemory;

    GraphicsBuffer() = default;

    GraphicsBuffer(VkBuffer buffer, VkDeviceMemory deviceMemory)
        : Handler(buffer), DeviceMemoryHandler(deviceMemory)
    {
        MappedMemory = nullptr;
    }
};

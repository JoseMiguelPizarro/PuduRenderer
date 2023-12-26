#pragma once
#include <optional>
#include <cstdint>
#include <PuduApp.h>

typedef std::optional<uint32_t> Optional;

struct QueueFamilyIndices {
	Optional graphicsFamily;
	Optional presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

const std::vector<const char*> DeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	PuduApp* app = reinterpret_cast<PuduApp*>(glfwGetWindowUserPointer(window));
	app->FrameBufferResized = true;
}
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class PuduApp
{
public:
	void Run();
	uint32_t WindowWidth = 800;
	uint32_t WindowHeight = 600;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
	


private:
	void InitVulkan();
	void CreateVulkanInstance();
	void MainLoop();
	void Cleanup();
	void InitWindow();
	bool CheckValidationLayerSupport();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void SetupDebugMessenger();
	std::vector<const char*> GetRequiredExtensions();

	GLFWwindow* m_windowPtr;
	VkInstance m_vkInstance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkAllocationCallbacks* m_allocatorPtr = nullptr;
};


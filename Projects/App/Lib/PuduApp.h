#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vector>

#include <VulkanManager.h>

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
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSurface();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateFrameBuffers();

	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	void MainLoop();
	void Cleanup();
	void InitWindow();
	bool CheckValidationLayerSupport();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void SetupDebugMessenger();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	

	std::vector<const char*> GetRequiredExtensions();
	std::vector<VkImageView> m_swapChainImagesViews;
	GLFWwindow* m_windowPtr;
	VkInstance m_vkInstance;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device;
	VkQueue m_graphicsQueue;
	VkQueue m_presentationQueue;
	VkSwapchainKHR m_swapChain;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImage> m_swapChainImages;
	VkRenderPass m_renderPass;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_graphicsPipeline;
	std::vector<VkFramebuffer> m_swapChainFrameBuffers;

	VkDebugUtilsMessengerEXT m_debugMessenger;



	VkAllocationCallbacks* m_allocatorPtr = nullptr;
};


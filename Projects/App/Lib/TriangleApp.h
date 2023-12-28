#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vector>

#include <VulkanManager.h>
#include <Frame.h>
#include <PuduApp.h>
#include <vertex.h>

class TriangleApp:PuduApp
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
	void MainLoop();
	void DrawFrame();
	void InitVulkan();

	void CreateVulkanInstance();
	void CreateVertexBuffer();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSurface();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateFrameBuffers();
	void CreateFrames();
	void CreateCommandPool();
	void CreateCommandBuffer();
	void CreateSyncObjects();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void RecreateSwapChain();

	void Cleanup();
	void CleanupSwapChain();

	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	void InitWindow();
	bool CheckValidationLayerSupport();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void SetupDebugMessenger();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	

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
	VkCommandPool m_commandPool;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;

	VkDebugUtilsMessengerEXT m_debugMessenger;

	std::vector<Frame> m_Frames;

	bool framebufferResized = false;
	const int MAX_FRAMES_IN_FLIGHT = 2;
	uint32_t m_currentFrame = 0;

	const std::vector<Vertex> m_vertices = {
	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

	VkAllocationCallbacks* m_allocatorPtr = nullptr;
};


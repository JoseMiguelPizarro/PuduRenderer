#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <optional>
#include <Frame.h>
#include <vertex.h>
#include <GraphicsBuffer.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

typedef std::optional<uint32_t> Optional;

using namespace glm;

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

class PuduGraphics
{
public:
	void Init(int windowWidth, int windowHeight);
	void DrawFrame();

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

	VkDevice Device;
	GLFWwindow* WindowPtr;
	bool FramebufferResized = false;
	void Cleanup();

private:
	void InitVulkan();
	void CreateVulkanInstance();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSurface();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateBuffers();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateFrameBuffers();
	void CreateFrames();
	void CreateCommandPool(VkCommandPool* cmdPool);

	void CreateTextureImage();
	void CreateTextureImageView();
	void CreateTextureSampler();

	void CreateDescriptorPool();
	void CreateDescriptorSets();
	void CreateCommandBuffer();
	void CreateSyncObjects();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void RecreateSwapChain();
	void UpdateUniformBuffer(uint32_t currentImage);

	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void InitImgui();
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	VkPipelineCache m_pipelineCache;
	void CleanupSwapChain();
	GraphicsBuffer CreateGraphicsBuffer(uint64_t size, void* bufferData, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	void CreateImage(uint32_t width, uint32_t height, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	VkImageView CreateImageView(VkImage image, VkFormat format);

	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	VkImageView m_textureImageView;
	VkSampler m_textureSampler;

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
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void DestroyBuffer(GraphicsBuffer buffer);
	std::vector<const char*> GetRequiredExtensions();
	std::vector<VkImageView> m_swapChainImagesViews;
	VkInstance m_vkInstance;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkQueue m_graphicsQueue;
	VkQueue m_presentationQueue;
	VkSwapchainKHR m_swapChain;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImage> m_swapChainImages;
	VkRenderPass m_renderPass;
	VkPipeline m_graphicsPipeline;
	VkPipeline m_imguiPipeline;
	std::vector<VkFramebuffer> m_swapChainFrameBuffers;
	VkCommandPool m_commandPool;

	GraphicsBuffer m_vertexBuffer;
	GraphicsBuffer m_indexBuffer;

	VkImage m_textureImage;
	VkDeviceMemory m_textureImageMemory;

	std::vector<GraphicsBuffer> m_uniformBuffers;

	VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_pipelineLayout;

	VkCommandPool m_ImGuiCommandPool;
	VkRenderPass m_ImGuiRenderPass;
	std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;
	std::vector<VkFramebuffer> m_ImGuiFramebuffers;
	void CreateImGuiRenderPass();
	void CreateImGUICommandBuffers();
	void CreateImGUIFrameBuffer();

	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSets;


	VkDebugUtilsMessengerEXT m_debugMessenger;

	std::vector<Frame> m_Frames;

	const int MAX_FRAMES_IN_FLIGHT = 2;
	uint32_t m_currentFrame = 0;
	uint32_t m_imageCount;

	bool m_initialized = false;

	std::vector<Vertex> m_vertices = {
	{vec3(-0.5f, -0.5f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f)},
	{vec3(0.5f, -0.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec2(1.0f, 0.0f) },
	{vec3(0.5f, 0.5f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 1.0f) },
	{vec3(-0.5f, 0.5f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 1.0f) }


	/*{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}*/
	};

	std::vector<uint16_t> m_indices = {
		0, 1, 2, 2, 3, 0,
		/*4, 5, 6, 6, 7, 4*/
	};

	VkAllocationCallbacks* m_allocatorPtr = nullptr;
};

void static FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	PuduGraphics* app = reinterpret_cast<PuduGraphics*>(glfwGetWindowUserPointer(window));
	app->FramebufferResized = true;
}



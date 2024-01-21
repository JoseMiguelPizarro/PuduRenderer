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

#include <optional>
#include <Frame.h>
#include <GraphicsBuffer.h>
#include <string>

#include "DrawCall.h"
#include "Mesh.h"
#include "FileManager.h"
#include "Texture2D.h"

#include "Camera.h"
#include "Scene.h"
#include "Model.h"
#include "UniformBufferObject.h"
#include "MeshCreationData.h"

namespace Pudu
{
	typedef std::optional<uint32_t> Optional;

	using namespace glm;

	struct QueueFamilyIndices
	{
		Optional graphicsFamily;
		Optional presentFamily;

		bool IsComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
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
		static PuduGraphics* Instance();
		void Init(int windowWidth, int windowHeight);
		void DrawFrame();

		uint32_t WindowWidth = 800;
		uint32_t WindowHeight = 600;

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		Scene* SceneToRender;

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif

		VkDevice Device;
		GLFWwindow* WindowPtr;
		bool FramebufferResized = false;
		void InitPipeline();
		void Cleanup();
		Model CreateModel(std::shared_ptr<Mesh> mesh, Material& material);
		Model CreateModel(MeshCreationData const& data);
		Mesh CreateMesh(MeshCreationData const& meshData);
		void DestroyMesh(Mesh& mesh);
		void DestroyTexture(Texture2d& texture);
		Texture2d CreateTexture(std::filesystem::path const& path);

		GraphicsBuffer CreateGraphicsBuffer(uint64_t size, void* bufferData, VkBufferUsageFlags usage,
			VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		void CreateImage(uint32_t width, uint32_t height, VkFormat format,
			VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	private:
		static PuduGraphics* s_instance;

		void InitVulkan();
		void CreateVulkanInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSurface();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateFrames();
		void CreateCommandPool(VkCommandPool* cmdPool);

		void CreateTextureImageView(Texture2d& texture2d);
		void CreateTextureSampler();

		void CreateDescriptorPool();
		void CreateDescriptorSets(Model* model);
		void CreateCommandBuffer();
		void CreateSyncObjects();
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void RecreateSwapChain();
		void UpdateUniformBuffer(uint32_t currentImage);
		UniformBufferObject GetUniformBufferObject(Camera& cam, DrawCall& drawCall);

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

#pragma region  ImGUI
		void InitImgui();
		VkCommandPool m_ImGuiCommandPool;
		VkRenderPass m_ImGuiRenderPass;
		VkPipeline m_imguiPipeline;
		VkDescriptorPool m_ImGuiDescriptorPool;
		std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;
		std::vector<VkFramebuffer> m_ImGuiFrameBuffers;

		void CreateImGuiRenderPass();
		void CreateImGUICommandBuffers();
		void CreateImGUIFrameBuffers();
#pragma endregion

#pragma region DepthBuffer
		VkImage m_depthImage;
		VkDeviceMemory m_depthImageMemory;
		VkImageView m_depthImageView;

		void CreateDepthResources();
		VkFormat FindDepthFormat();

		bool HasStencilComponent(VkFormat format);
#pragma endregion

		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
			VkFormatFeatureFlags features);

		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

		VkPipelineCache m_pipelineCache;
		void CleanupSwapChain();

		

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		VkShaderModule CreateShaderModule(const std::vector<char>& code);


		VkSampler m_textureSampler;

		void CreateUniformBuffers();

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
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);
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

		std::vector<VkFramebuffer> m_swapChainFrameBuffers;
		VkCommandPool m_commandPool;



		std::vector<GraphicsBuffer> m_uniformBuffers;

		VkDescriptorSetLayout m_descriptorSetLayout;
		VkPipelineLayout m_pipelineLayout;

		VkDescriptorPool m_descriptorPool;

		VkDebugUtilsMessengerEXT m_debugMessenger;

		std::vector<Frame> m_Frames;

		const int MAX_FRAMES_IN_FLIGHT = 2;
		uint32_t m_currentFrame = 0;
		uint32_t m_imageCount;

		bool m_initialized = false;

		VkAllocationCallbacks* m_allocatorPtr = nullptr;
	};

	void static FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		PuduGraphics* app = reinterpret_cast<PuduGraphics*>(glfwGetWindowUserPointer(window));
		app->FramebufferResized = true;
	}
}

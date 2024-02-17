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
#include "Texture2D.h"

#include "Camera.h"
#include "Scene.h"
#include "Model.h"
#include "UniformBufferObject.h"
#include "MeshCreationData.h"
#include <PhysicalDeviceCreationData.h>
#include <ResourceUpdate.h>
#include <GPUResourcesManager.h>
#include <PipelineCreationData.h>
#include <Resources/Resources.h>
#include <Resources/RenderPassCreationData.h>
#include <Resources/FrameBufferCreationData.h>
#include <FrameGraph/FrameGraphRenderPass.h>
#include <RenderFrameData.h>
#include "GPUCommands.h"

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
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		VK_KHR_MAINTENANCE3_EXTENSION_NAME,
		// Works around a validation layer bug with descriptor pool allocation with VARIABLE_COUNT.
	// See: https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2350.
		VK_KHR_MAINTENANCE1_EXTENSION_NAME
	};

	class PuduGraphics
	{
	public:
		static PuduGraphics* Instance();
		void Init(int windowWidth, int windowHeight);
		void DrawFrame(RenderFrameData& frameData);

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

		GLFWwindow* WindowPtr;
		bool FramebufferResized = false;
		void InitPipeline();
		void Cleanup();
		Model CreateModel(std::shared_ptr<Mesh> mesh, Material& material);
		Model CreateModel(MeshCreationData const& data);
		Mesh CreateMesh(MeshCreationData const& meshData);
		void DestroyMesh(Mesh& mesh);
		void DestroyTexture(Texture2d& texture);
		void WaitIdle();

		void DestroyRenderPass(RenderPassHandle handle);
		void DestroyFrameBuffer(FramebufferHandle handle);

		SPtr<GPUResourcesManager> m_resources = nullptr;

		void CreateVkRenderPass();

		/// <summary>
		/// Creates a vkRenderPass and attach it to the passed RenderPass object
		/// </summary>
		/// <param name="renderPass"></param>
		void CreateVkRenderPass(RenderPass* renderPass);
		void CreateVkFramebuffer(Framebuffer* creationData);

		GraphicsBuffer CreateGraphicsBuffer(uint64_t size, void* bufferData, VkBufferUsageFlags usage,
			VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		void CreateImage(uint32_t width, uint32_t height, VkFormat format,
			VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

		GPUResourcesManager& GetResources() {
			return *m_resources;
		}

		PipelineHandle CreateGraphicsPipeline(PipelineCreationData& creationData);
		VkPipeline* GetPipeline() { return &m_graphicsPipeline; }

		
		void DrawImGui(RenderFrameData& frameData);
		void SubmitFrame(RenderFrameData& frameData);
		void EndDrawFrame();
		UniformBufferObject GetUniformBufferObject(Camera& cam, DrawCall& drawCall);
		Shader* CreateShader(char* fragmentPath, char* vertexPath);

		TextureHandle CreateTexture(TextureCreationData const& creationData);
		SPtr<Texture2d> CreateTexture(std::filesystem::path const& path);

	private:
		friend class GPUResourcesManager;

		static PuduGraphics* s_instance;
		static uint32_t const k_MAX_BINDLESS_RESOURCES = 100; //100 idkw
		static uint32_t const k_BINDLESS_TEXTURE_BINDING = 32; //32 idkw
		std::vector<ResourceUpdate> m_bindlessResourcesToUpdate;
		VkDevice m_device;

		GPUCommands m_commandBuffer;
		uint32_t m_imageIndex;

		
		void InitVulkan();
		void InitVMA();
		void CreateVulkanInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSurface();
		void CreateSwapChain();
		void CreateImageViews();
		
		ShaderStateHandle CreateShaderState(ShaderStateCreationData const& creation);
		std::vector<DescriptorSetLayoutHandle> CreateDescriptorSetLayout(std::vector<DescriptorSetLayoutData>& creationData);

		void CreateFrameBuffers();
		void CreateFrames();
		void CreateCommandPool(VkCommandPool* cmdPool);

		void CreateTextureImageView(Texture2d& texture2d);
		void CreateTextureSampler(VkSampler& sampler);

		void CreateBindlessDescriptorPool();
		void CreateBindlessDescriptorSet(VkDescriptorSet& descriptorSet, VkDescriptorSetLayout* layouts);
		void CreateCommandBuffer();
		void CreateSyncObjects();
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void RecreateSwapChain();
		void UpdateUniformBuffer(uint32_t currentImage);
		
		void UpdateBindlessResources();

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);


		VmaAllocator m_VmaAllocator;

		PhysicalDeviceCreationData m_physicalDeviceData;
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

		VkShaderModule CreateShaderModule(char const* code, size_t size);

		void CreateUniformBuffers();

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		void InitWindow();
		bool CheckValidationLayerSupport();
		void UpdateBindlessTexture(TextureHandle texture);
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
		std::vector<const char*> GetInstanceExtensions();
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

		VkDescriptorSet m_bindlessDescriptorSet;
		VkPipelineLayout m_pipelineLayout;

		VkDescriptorPool m_descriptorPool;

		VkDebugUtilsMessengerEXT m_debugMessenger;

		std::vector<Frame> m_Frames;

		const int MAX_FRAMES_IN_FLIGHT = 2;
		uint32_t m_currentFrameIndex = 0;
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

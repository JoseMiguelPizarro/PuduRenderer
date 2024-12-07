#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vector>

#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "vma/vk_mem_alloc.h"

#include <optional>
#include <Frame.h>
#include <GraphicsBuffer.h>
#include <string>

#include "DrawCall.h"
#include "Mesh.h"
#include "Texture.h"
#include "Texture2D.h"
#include "TextureCube.h"

#include "Camera.h"
#include "Scene.h"
#include "Model.h"
#include "UniformBufferObject.h"
#include "MeshCreationData.h"
#include <PhysicalDeviceCreationData.h>
#include <ResourceUpdate.h>
#include <GPUResourcesManager.h>
#include <Resources/Resources.h>
#include "FrameGraph/RenderPass.h"
#include <Resources/FrameBufferCreationData.h>
#include <RenderFrameData.h>
#include "GPUCommands.h"
#include <ComputeShader.h>
#include "Pipeline.h"
#include "Resources/CommandPool.h"


namespace Pudu
{
	typedef std::optional<uint32_t> Optional;

	using namespace glm;


	enum QueueFamily {
		Graphics,
		Compute,
		Transfer,
		Present
	};

	struct QueueFamilyIndices
	{
		Optional graphicsFamily;
		Optional presentFamily;
		Optional computeFamily;
		Optional transferFamily;

		bool IsComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value() && transferFamily.has_value();
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
		VK_KHR_MAINTENANCE1_EXTENSION_NAME,
		VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
		VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
	};

	static uint32_t const K_BINDLESS_SET_INDEX = 0;
	static uint32_t const k_MAX_BINDLESS_RESOURCES = 100; //100 idkw
	static uint32_t const k_BINDLESS_TEXTURE_BINDING = 32; //32 idkw
	static uint32_t const K_LIGHTING_BUFFER_BINDING = 0; //32 idkw

	class PuduGraphics
	{
	public:
		static PuduGraphics* Instance();
		void Init(int windowWidth, int windowHeight);
		void DrawFrame(RenderFrameData& frameData);

		uint32_t WindowWidth = 1280;
		uint32_t WindowHeight = 800;

		const std::vector<const char*> validationLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif

		GLFWwindow* WindowPtr;
		bool FramebufferResized = false;
		void Cleanup();
		Model CreateModel(std::shared_ptr<Mesh> mesh, Material& material);
		Model CreateModel(MeshCreationData const& data);
		SPtr<Mesh> CreateMesh(MeshCreationData const& meshData);
		void UpdateDescriptorSet(uint16_t count, VkWriteDescriptorSet* write, uint16_t copyCount = 0, const VkCopyDescriptorSet* copy = nullptr);
		void DestroyMesh(SPtr<Mesh> mesh);
		void DestroyTexture(SPtr<Texture> texture);
		void WaitIdle();

		void DestroyRenderPass(SPtr<RenderPass> renderPass);
		void DestroyFrameBuffer(SPtr<Framebuffer> frameBuffer);


		/// <summary>
		/// Creates a vkRenderPass and attach it to the passed RenderPass object
		/// </summary>
		/// <param name="renderPass"></param>
		void CreateRenderPass(RenderPass* creationData);

		template<class T>
			requires (std::convertible_to<T, RenderPass>)
		SPtr<T> GetRenderPass() {
			return m_resources.AllocateRenderPass<T>();
		}

		SPtr<Framebuffer> CreateFramebuffer(FramebufferCreationData const& creationData);

		SPtr<GraphicsBuffer> CreateGraphicsBuffer(uint64_t size, void* bufferData, VkBufferUsageFlags usage,
			VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, const char* name = nullptr);

		//TODO: ADD SUPPORT FOR MIPMAPS

		struct ImageCreateData {
			uint32_t width;
			uint32_t height;
			uint32_t depth = 1;
			uint32_t mipLevels = 1;
			uint32_t arrayLayers = 1;
			VkFormat format;
			VkImageTiling tilling;
			VkImageUsageFlags usage;
			VkMemoryPropertyFlags memoryPropertiesFlags;
			VkImage* image;
			VkDeviceMemory* imageMemory;
			char* name;
		};

		struct ImageViewCreateData
		{
			VkImage image;
			VkFormat format;
			VkImageViewType imageView = VK_IMAGE_VIEW_TYPE_2D;
			VkImageAspectFlags aspectFlags;
			VkImageViewType viewType;
			uint32_t levelCount = 1;
			uint32_t baseMipLevel = 0;
			uint32_t baseArrayLayer = 0;
			uint32_t layerCount = 1;
			const char* name;
		};

		GPUResourcesManager* Resources() {
			return &m_resources;
		}

		GPUResourceHandle<Pipeline> CreateGraphicsPipeline(PipelineCreationData& creationData);
		GPUResourceHandle<Pipeline> CreateComputePipeline(ComputePipelineCreationData& creationData);


		void DrawImGui(RenderFrameData& frameData);
		void SubmitFrame(RenderFrameData& frameData);
		void EndDrawFrame();
		UniformBufferObject GetUniformBufferObject(Camera* cam, DrawCall& drawCall);
		SPtr<Shader> CreateShader(fs::path fragmentPath, fs::path vertexPath, const char* name);
		SPtr<ComputeShader> CreateComputeShader(fs::path shaderPath, const char* name);

		SPtr<RenderTexture> GetRenderTexture();
		SPtr<Texture2d> LoadTexture2D(fs::path filePath, TextureCreationSettings& creationData);
		SPtr<TextureCube> LoadTextureCube(fs::path filePath, TextureCreationSettings& creationSettings);
		GPUResourceHandle<Texture> CreateTexture(TextureCreationData const& creationData);
		void CreateVKTexture(Texture* texture);
		void CreateVKTextureSampler(SamplerCreationData data, VkSampler& sampler);

		void UploadTextureData(Texture* texture, void* data, VkImageSubresourceRange& range, std::vector<VkBufferImageCopy2>* regions = nullptr);

		void UpdateBindlessResources(Pipeline* pipeline);
		SPtr<CommandPool> GetCommandPool(QueueFamily type);


	private:
		SPtr<Texture> LoadAndCreateTexture(fs::path filePath, TextureCreationSettings& creationData);
		friend class GPUResourcesManager;

		static PuduGraphics* s_instance;

		std::vector<ResourceUpdate> m_bindlessResourcesToUpdate;
		VkDevice m_device;

		GPUResourcesManager m_resources;
		SPtr<Semaphore> m_graphicsTimelineSemaphore;
		SPtr<Semaphore> m_computeTimelineSemaphore;
		uint64_t m_lastComputeTimelineValue = 0;

		PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT;

		void InitVulkan();
		void InitVMA();
		void CreateVulkanInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSurface();
		void CreateSwapChain();
		void CreateSwapchainImageViews();
		void SetResourceName(VkObjectType type, u64 handle, const char* name);
		/// <summary>
		/// Setup and dispatch compute workload for the frame
		/// </summary>
		void SubmitComputeWork(RenderFrameData& frameData);
		void UpdateComputeResources(ComputeShader* shader);

		void InitDebugUtilsObjectName();

		GPUResourceHandle<ShaderState> CreateShaderState(ShaderStateCreationData const& creation);
		GPUResourceHandle<DescriptorSetLayout> CreateBindlessDescriptorSetLayout(DescriptorSetLayoutData& creationData);
		GPUResourceHandle<DescriptorSetLayout> CreateDescriptorSetLayout(DescriptorSetLayoutData& creationData);
		void CreateDescriptorsLayouts(std::vector<DescriptorSetLayoutData>& layoutData, std::vector<GPUResourceHandle<DescriptorSetLayout>>& out);

		void CreateBindlessDescriptorPool();
		void CreateFrames();
		void CreateCommandPool(VkCommandPool* cmdPool, uint32_t familyIndex);

		void CreateTextureImageView(Texture2d& texture2d);
		SPtr<Semaphore> CreateTimelineSemaphore(const char* name = nullptr);
		SPtr<Semaphore> CreateSemaphoreSPtr(const char* name = nullptr);
		std::vector<SPtr<GPUCommands>> CreateCommandBuffers(GPUCommands::CreationData creationData, const char* name = nullptr);

		void DestroySemaphore(SPtr<Semaphore> semaphore);
		void DestroyShader(SPtr<Shader> shader);
		void DestroyShaderModule(VkShaderModule& state);
		void DestroyDescriptorSetLayout(DescriptorSetLayout& descriptorset);
		void DestroyCommandPool(CommandPool* commandPool);
		void CreateDescriptorSets(VkDescriptorPool pool, VkDescriptorSet* descriptorSet, uint16_t setsCount, VkDescriptorSetLayout* layouts, uint32_t layoutsCount);
		void CreateFramesCommandBuffer();
		void CreateSwapChainSyncObjects();
		void RecreateSwapChain();
		void UpdateUniformBuffer(uint32_t currentImage);

		void UpdateLightingBuffer(RenderFrameData& frame);

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, std::vector<VkBufferImageCopy2>* regions = nullptr);


		VmaAllocator m_VmaAllocator;

		PhysicalDeviceCreationData m_physicalDeviceData;
#pragma region  ImGUI
		void InitImgui();
		VkCommandPool m_ImGuiCommandPool;
		SPtr<RenderPass> m_ImGuiRenderPass;
		VkPipeline m_imguiPipeline;
		VkDescriptorPool m_ImGuiDescriptorPool;
		std::vector<SPtr<GPUCommands>> m_ImGuiCommandBuffers;
		std::vector<SPtr<Framebuffer>> m_ImGuiFrameBuffers;

		void CreateImGuiRenderPass();
		void CreateImGUICommandBuffers();
		void CreateImGUIFrameBuffers();
#pragma endregion
#pragma region DepthBuffer
		void CreateDepthResources();
		VkFormat FindDepthFormat();

		bool HasStencilComponent(VkFormat format);
#pragma endregion

		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
			VkFormatFeatureFlags features);

		GPUCommands BeginSingleTimeCommands();
		void EndSingleTimeCommands(GPUCommands commandBuffer);

		VkPipelineCache m_pipelineCache;
		void CleanupSwapChain();

		VkImageView CreateImageView(ImageViewCreateData data);


		/// <summary>
		/// size in bytes
		/// </summary>
		VkShaderModule CreateShaderModule(const std::vector<char>& code, size_t size, const char* name = nullptr);

		void CreateUniformBuffers();
		void CreateLightingBuffers();


		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		void InitWindow();
		bool CheckValidationLayerSupport();
		void UpdateBindlessTexture(GPUResourceHandle<Texture> handle);
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void SetupDebugMessenger();
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VmaAllocation CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateFlags properties,
			VkBuffer& buffer, const char* name = nullptr);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void DestroyBuffer(SPtr<GraphicsBuffer> buffer);
		void AdvanceFrame();
		std::vector<const char*> GetInstanceExtensions();
		std::vector<VkImageView> m_swapChainImagesViews;
		VkFormat m_swapChainImageFormat;
		VkSwapchainKHR m_swapChain;
		VkInstance m_vkInstance;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkQueue m_graphicsQueue;
		VkQueue m_presentationQueue;
		VkQueue m_computeQueue;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImage> m_swapChainImages;
		std::vector<SPtr<RenderTexture>> m_swapChainTextures;

		SPtr<CommandPool> m_commandPool;

		std::vector<SPtr<GraphicsBuffer>> m_uniformBuffers;
		std::vector<SPtr<GraphicsBuffer>> m_lightingBuffers;

		VkDescriptorPool m_bindlessDescriptorPool;

		VkDebugUtilsMessengerEXT m_debugMessenger;

		std::vector<Frame> m_Frames;

		const int MAX_FRAMES_IN_FLIGHT = 2;
		uint32_t m_currentFrameIndex = 0;
		uint32_t m_imageCount;
		uint64_t m_absoluteFrame = 1;

		bool m_initialized = false;

		VkAllocationCallbacks* m_allocatorPtr = nullptr;
	};



	void static FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		PuduGraphics* app = reinterpret_cast<PuduGraphics*>(glfwGetWindowUserPointer(window));
		app->FramebufferResized = true;
	}
}

//Windows defines a min max func that messes up std funcs :') 
#define NOMINMAX 

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <fmt/core.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <limits>

#include "PuduGraphics.h"
#include <PuduGlobals.h>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <Logger.h>
#include <set>
#include "UniformBufferObject.h"
#include <chrono>
#include <PuduMath.h>

#include "FileManager.h"

#include "DrawCall.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_vulkan.h"

#include "ImGuiUtils.h"
#include "SPIRVParser.h"
#include "Frame.h"
#include "FrameGraph/FrameGraph.h"

#include "VulkanUtils.h"
#include "PuduApp.h"
#include "Pipeline.h"
#include "Lighting/LightBuffer.h"
#include "Texture.h"
#include "Texture2D.h"
#include "TextureCube.h"

#include <ktx.h>
#include <ktxvulkan.h>

namespace Pudu
{
	const char* SHADER_ENTRY_POINT = "main";

	struct FrameGraph;

	PuduGraphics* PuduGraphics::s_instance = nullptr;

	PuduGraphics* PuduGraphics::Instance()
	{
		return s_instance;
	}

	void PuduGraphics::Init(int windowWidth, int windowHeight)
	{
		PuduGraphics::s_instance = this;
		LOG("Graphics Init");
		WindowWidth = windowWidth;
		WindowHeight = windowHeight;

		m_resources.Init(this);

		InitWindow();
		InitVulkan();
		InitImgui();

		m_initialized = true;
	}

	void PuduGraphics::InitVulkan()
	{
		CreateFrames();
		CreateVulkanInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		InitDebugUtilsObjectName();
		InitVMA();
		CreateSwapChain();
		CreateSwapchainImageViews();

		CreateBindlessDescriptorPool();

		CreateCommandPool(&m_commandPool);
		CreateUniformBuffers();
		CreateLightingBuffers();

		CreateFramesCommandBuffer();
		CreateSwapChainSyncObjects();

		testComputeShader = CreateComputeShader("Shaders/shadows.comp", "ShadowsCS");
	}

	void PuduGraphics::InitVMA()
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = m_physicalDevice;
		allocatorInfo.device = m_device;
		allocatorInfo.instance = m_vkInstance;

		vmaCreateAllocator(&allocatorInfo, &m_VmaAllocator);
	}

	void PuduGraphics::InitWindow()
	{
		Print("Graphics Init Window");
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		WindowPtr = glfwCreateWindow(WindowWidth, WindowHeight, RENDERER_NAME, nullptr, nullptr);
		glfwSetWindowUserPointer(WindowPtr, this);
		glfwSetFramebufferSizeCallback(WindowPtr, FramebufferResizeCallback);
	}

	bool PuduGraphics::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	void PuduGraphics::UpdateBindlessTexture(TextureHandle texture)
	{
		ResourceUpdate resourceToUpdate{};
		resourceToUpdate.handle = { texture.index };

		m_bindlessResourcesToUpdate.push_back(resourceToUpdate);
	}

	void PuduGraphics::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		createInfo.pfnUserCallback = VkDebugCallback;
		createInfo.pUserData = nullptr;
	}

	void PuduGraphics::SetupDebugMessenger()
	{
		if (!enableValidationLayers)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, m_allocatorPtr, &m_debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set up debug messenger!");
		}
	}

	VkSurfaceFormatKHR PuduGraphics::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace ==
				VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR PuduGraphics::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D PuduGraphics::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		int width, height;
		glfwGetFramebufferSize(WindowPtr, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height);

		return actualExtent;
	}

	void PuduGraphics::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory, const char* name)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(m_device, buffer, bufferMemory, 0);

		if (name != nullptr)
		{
			SetResourceName(VK_OBJECT_TYPE_BUFFER, (glm::u64)buffer, name);
		}
	}

	void PuduGraphics::UpdateDescriptorSet(uint16_t count, VkWriteDescriptorSet* write, uint16_t copyCount, const VkCopyDescriptorSet* copy)
	{
		vkUpdateDescriptorSets(m_device, count, write, copyCount, copy);
	}

	void PuduGraphics::UpdateLightingBuffer(RenderFrameData& frame)
	{
		auto buffer = m_lightingBuffers[frame.frameIndex];

		LightBuffer lightBuffer{};
		lightBuffer.lightDirection = { -frame.scene->directionalLight->Direction(),0.0f };
		lightBuffer.dirLightMatrix = frame.scene->directionalLight->GetLightMatrix();
		lightBuffer.shadowMatrix = frame.scene->directionalLight->GetShadowMatrix();

		memcpy(buffer->MappedMemory, &lightBuffer, sizeof(LightBuffer));

		frame.lightingBuffer = m_lightingBuffers[frame.frameIndex];
	}

	void PuduGraphics::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		auto commandBuffer = BeginSingleTimeCommands();

		commandBuffer.CopyBuffer(srcBuffer, dstBuffer, size);

		EndSingleTimeCommands(commandBuffer);
	}

	uint32_t PuduGraphics::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		PUDU_ERROR("Failed to find suitable memory type!");
	}

	void PuduGraphics::DestroyBuffer(SPtr<GraphicsBuffer> buffer)
	{
		if (!buffer->IsDestroyed())
		{
			vkDestroyBuffer(m_device, buffer->vkHandler, m_allocatorPtr);
			vkFreeMemory(m_device, buffer->DeviceMemoryHandler, m_allocatorPtr);
			buffer->Destroy();
		}
	}

	std::vector<const char*> PuduGraphics::GetInstanceExtensions()
	{
		uint32_t glfwExtensionsCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		return extensions;
	}




	void PuduGraphics::CreateImGuiRenderPass()
	{
		VkAttachmentDescription attachment = {};
		attachment.format = m_swapChainImageFormat;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachment = {};
		colorAttachment.attachment = 0;
		colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachment;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0; // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;

		if (vkCreateRenderPass(m_device, &info, nullptr, &m_ImGuiRenderPass) != VK_SUCCESS)
			throw std::runtime_error("failed to create render pass!");
	}

	void PuduGraphics::CreateImGUICommandBuffers()
	{
		m_ImGuiCommandBuffers.resize(m_swapChainImagesViews.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_ImGuiCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_ImGuiCommandBuffers.size();

		if (vkAllocateCommandBuffers(m_device, &allocInfo, m_ImGuiCommandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void PuduGraphics::CreateImGUIFrameBuffers()
	{
		LOG("Create ImGUI FrameBuffers");
		m_ImGuiFrameBuffers.resize(m_swapChainImagesViews.size());

		for (size_t i = 0; i < m_swapChainImagesViews.size(); i++)
		{
			std::array<VkImageView, 1> attachments = {
				m_swapChainImagesViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_ImGuiRenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_ImGuiFrameBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
		Print("Created frame buffers");
	}

	void PuduGraphics::DrawFrame(RenderFrameData& frameData)
	{
		auto frameGraph = frameData.frameGraph;
		Frame frame = m_Frames[m_currentFrameIndex];

		//don't wait the first frames
		if (m_absoluteFrame >= MAX_FRAMES_IN_FLIGHT)
		{
			uint64_t graphicsTimelineValue = m_absoluteFrame;
			uint64_t computeTimelineValue = m_lastComputeTimelineValue;

			uint64_t timelineValues[]{ graphicsTimelineValue,computeTimelineValue };
			VkSemaphore semaphores[]{ m_graphicsTimelineSemaphore->vkHandle, m_computeTimelineSemaphore->vkHandle };

			VkSemaphoreWaitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
			waitInfo.semaphoreCount = 1; //for now just wait for the graphics
			waitInfo.pSemaphores = semaphores;
			waitInfo.pValues = timelineValues;

			vkWaitSemaphores(m_device, &waitInfo, ~0ull); //wait infinite
		}

		UpdateLightingBuffer(frameData);

		////Fences are used to ensure that the GPU has stopped using resources for a given frame. This force the CPU to wait for the GPU to finish using the resources
		//vkWaitForFences(m_device, 1, &frame.InFlightFence, VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, *frame.ImageAvailableSemaphore,
			VK_NULL_HANDLE, &frameData.frameIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			PUDU_ERROR("failed to acquire swap chain image!");
		}

		vkResetFences(m_device, 1, &frame.InFlightFence);

		frame.CommandBuffer.Reset();

		frame.CommandBuffer.BeginCommands();

		frameData.frame = &m_Frames[m_currentFrameIndex];
		frameData.currentCommand = &frame.CommandBuffer;
		frameData.graphics = this;
		frameData.camera = frameData.scene->camera;

		frameData.commandsToSubmit.push_back(frame.CommandBuffer.vkHandle);

		frameGraph->RenderFrame(frameData);

		DrawImGui(frameData);

		frameData.currentCommand->TransitionImageLayout(frameData.activeRenderTarget->vkImageHandle, frameData.activeRenderTarget->format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		frameData.currentCommand->TransitionImageLayout(m_swapChainTextures[frameData.frameIndex]->vkImageHandle, m_swapChainImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		frameData.currentCommand->Blit(frameData.activeRenderTarget, m_swapChainTextures[frameData.frameIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		frameData.currentCommand->TransitionImageLayout(m_swapChainTextures[frameData.frameIndex]->vkImageHandle, m_swapChainImageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		frame.CommandBuffer.EndCommands();

		auto computeCommands = &frame.ComputeCommandBuffer;
		frameData.computeCommandsToSubmit.push_back(computeCommands);
		computeCommands->Reset();

		if (!testComputeShader->ResourcesUpdated())
		{
			UpdateComputeResources(testComputeShader.get());
		}

		//auto computePipeline = m_resources.GetPipeline(testComputeShader->pipelineHandle);
		//computeCommands->BeginCommands();
		////IMPORTANT: Descriptor sets should be binded BEFORE binding the pipeline
		///*computeCommands->BindDescriptorSetCompute(computePipeline->vkPipelineLayoutHandle, &computePipeline->vkDescriptorSet, 1);
		//computeCommands->BindPipeline(m_resources.GetPipeline(testComputeShader->pipelineHandle));
		//computeCommands->Dispatch(std::ceil(1024 / 16.0f), std::ceil(1024 / 16.0f), 1);*/
		//computeCommands->EndCommands();

		SubmitComputeWork(frameData);
		SubmitFrame(frameData);

		EndDrawFrame();
	}

	void PuduGraphics::SubmitComputeWork(RenderFrameData& frameData)
	{
		auto frame = frameData.frame;

		bool hasWaitSemaphore = m_lastComputeTimelineValue > 0;

		VkSemaphoreSubmitInfo waitSemaphores[]{
			{
			VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, m_computeTimelineSemaphore->vkHandle, m_lastComputeTimelineValue, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,0 },
		};

		m_lastComputeTimelineValue++;

		VkSemaphoreSubmitInfo signalSemaphores[]{
			{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, m_computeTimelineSemaphore->vkHandle, m_lastComputeTimelineValue, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,0 }
		};

		VkCommandBufferSubmitInfo commandSubmitInfo;


		std::vector<VkCommandBufferSubmitInfo> commandSubmitInfos;
		for (auto command : frameData.computeCommandsToSubmit) {
			if (!command->HasRecordedCommand())
			{
				continue;
			}

			VkCommandBufferSubmitInfo commandInfo{};
			commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			commandInfo.commandBuffer = command->vkHandle;

			commandSubmitInfos.push_back(commandInfo);
		}

		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.waitSemaphoreInfoCount = hasWaitSemaphore ? 1 : 0;
		submitInfo.pWaitSemaphoreInfos = waitSemaphores;
		submitInfo.signalSemaphoreInfoCount = 1;
		submitInfo.pSignalSemaphoreInfos = signalSemaphores;
		submitInfo.commandBufferInfoCount = commandSubmitInfos.size();
		submitInfo.pCommandBufferInfos = commandSubmitInfos.data();

		vkQueueSubmit2(m_computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
	}

	void PuduGraphics::SubmitFrame(RenderFrameData& frameData)
	{
		auto frame = frameData.frame;

		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

		VkSemaphoreSubmitInfo waitSemaphores[]{
			{
			VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, *frame->ImageAvailableSemaphore,0, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,0 },
			{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, m_computeTimelineSemaphore->vkHandle, m_lastComputeTimelineValue, VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT,0 },
			{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, m_graphicsTimelineSemaphore->vkHandle, m_absoluteFrame - (MAX_FRAMES_IN_FLIGHT - 1), VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,0 },
		};

		VkSemaphoreSubmitInfo signalSemaphores[]{
			{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,nullptr,*frame->RenderFinishedSemaphore,0,VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,0},
			{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,nullptr,m_graphicsTimelineSemaphore->vkHandle, m_absoluteFrame + 1,VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT }
		};

		VkCommandBufferSubmitInfo commandSubmitInfo;

		std::vector<VkCommandBufferSubmitInfo> commandSubmitInfos;
		for (auto command : frameData.commandsToSubmit) {
			VkCommandBufferSubmitInfo commandInfo{};
			commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			commandInfo.commandBuffer = command;

			commandSubmitInfos.push_back(commandInfo);
		}

		VkSemaphore presentWaitSemaphores[]{
			*frame->RenderFinishedSemaphore
		};

		submitInfo.waitSemaphoreInfoCount = 2;
		submitInfo.pWaitSemaphoreInfos = waitSemaphores;

		submitInfo.signalSemaphoreInfoCount = 2;
		submitInfo.pSignalSemaphoreInfos = signalSemaphores;

		submitInfo.commandBufferInfoCount = frameData.commandsToSubmit.size();
		submitInfo.pCommandBufferInfos = commandSubmitInfos.data();


		VkResult submitResult = vkQueueSubmit2(m_graphicsQueue, 1, &submitInfo, frame->InFlightFence);
		if (submitResult != VK_SUCCESS)
		{
			LOG("VkERROR: {}", (uint32_t)submitResult);
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = presentWaitSemaphores;


		VkSwapchainKHR swapChains[] = { m_swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &frameData.frameIndex;
		presentInfo.pResults = nullptr; // Optional


		auto result = vkQueuePresentKHR(m_presentationQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || FramebufferResized)
		{
			FramebufferResized = false;
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swap chain image!");
		}
	}

	void PuduGraphics::AdvanceFrame()
	{
		//When `MAX_FRAMES_IN_FLIGHT` is a power of 2 you can update the current frame without modulo division
		m_currentFrameIndex = (m_currentFrameIndex + 1) & (MAX_FRAMES_IN_FLIGHT - 1);
		m_absoluteFrame++;
	}

	void PuduGraphics::EndDrawFrame()
	{
		AdvanceFrame();
	}

	void PuduGraphics::DrawImGui(RenderFrameData& frameData)
	{
		//ImGui Pass
		if (true) {
			VkRenderPassBeginInfo imGuiRenderPassInfo = {};
			imGuiRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			imGuiRenderPassInfo.renderPass = m_ImGuiRenderPass;

			imGuiRenderPassInfo.framebuffer = m_ImGuiFrameBuffers[frameData.frameIndex];
			imGuiRenderPassInfo.renderArea.offset = { 0, 0 };
			imGuiRenderPassInfo.renderArea.extent = m_swapChainExtent;
			VkClearValue clearColor = { 0.886f, 1.0f, 0.996f, 1.0f };
			imGuiRenderPassInfo.clearValueCount = 1;
			imGuiRenderPassInfo.pClearValues = &clearColor;

			// vkResetCommandPool(Device, m_ImGuiCommandPool, 0);
			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(m_ImGuiCommandBuffers[m_currentFrameIndex], &info);

			vkCmdBeginRenderPass(m_ImGuiCommandBuffers[m_currentFrameIndex], &imGuiRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();

			ImGui::NewFrame();
			ImGui::Begin("Pudu Renderer Debug");


			//Tree begin

			frameData.app->DrawImGUI();



			ImGui::End();
			ImGui::Render();

			// Record dear imgui primitives into command buffer
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ImGuiCommandBuffers[m_currentFrameIndex]);

			vkCmdEndRenderPass(m_ImGuiCommandBuffers[m_currentFrameIndex]);
			vkEndCommandBuffer(m_ImGuiCommandBuffers[m_currentFrameIndex]);

			ImGui::EndFrame();

			frameData.commandsToSubmit.push_back(m_ImGuiCommandBuffers[m_currentFrameIndex]);
		}
	}

	void PuduGraphics::CreateVulkanInstance()
	{
		if (enableValidationLayers && !CheckValidationLayerSupport())
		{
			throw std::runtime_error("Validation layers requested, but not available :(");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Pudu Renderer";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		//Add validation layers
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		auto extensions = GetInstanceExtensions();

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();


		//Enable debug info, this is necessary for spotting problems during create VKCreateInstance and VKDestroyInstance
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, m_allocatorPtr, &m_vkInstance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create instance!");
		}
		else
		{
			printf("Vulkan instance created successfully\n");
		}
	}

	void PuduGraphics::CreateVkFramebuffer(Framebuffer* framebuffer)
	{
		auto renderPass = m_resources.GetRenderPass(framebuffer->renderPassHandle);

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass->vkHandle;
		framebufferInfo.width = framebuffer->width;
		framebufferInfo.height = framebuffer->height;
		framebufferInfo.layers = 1;

		VkImageView framebufferAttachments[K_MAX_IMAGE_OUTPUTS + 1]{};
		uint32_t activeAttachments = 0;
		for (; activeAttachments < framebuffer->numColorAttachments; activeAttachments++)
		{
			auto texture = m_resources.GetTexture<Texture2d>(framebuffer->colorAttachmentHandles[activeAttachments]);
			framebufferAttachments[activeAttachments] = texture->vkImageViewHandle;
		}

		if (framebuffer->depthStencilAttachmentHandle.index != k_INVALID_HANDLE)
		{
			auto depthTexture = m_resources.GetTexture<Texture2d>(framebuffer->depthStencilAttachmentHandle);
			framebufferAttachments[activeAttachments++] = depthTexture->vkImageViewHandle;
		}

		framebufferInfo.attachmentCount = activeAttachments;
		framebufferInfo.pAttachments = framebufferAttachments;

		vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &framebuffer->vkHandle);

		//TODO: SET RESOURCE NAME
	}

	SPtr<GraphicsBuffer> PuduGraphics::CreateGraphicsBuffer(uint64_t size, void* bufferData, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags flags, const char* name)
	{
		VkDeviceSize bufferSize = size;
		VkBuffer vkBuffer = {};
		VkDeviceMemory vkdeviceMemory = {};

		if (bufferData != nullptr)
		{
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				stagingBufferMemory);

			void* data;
			vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, bufferData, (size_t)bufferSize);
			vkUnmapMemory(m_device, stagingBufferMemory);

			CreateBuffer(bufferSize, usage, flags, vkBuffer,
				vkdeviceMemory, name);

			CopyBuffer(stagingBuffer, vkBuffer, bufferSize);

			vkDestroyBuffer(m_device, stagingBuffer, nullptr);
			vkFreeMemory(m_device, stagingBufferMemory, nullptr);
		}
		else
		{
			CreateBuffer(bufferSize, usage, flags, vkBuffer,
				vkdeviceMemory, name);
		}

		auto graphicsBuffer = m_resources.AllocateGraphicsBuffer();
		graphicsBuffer->vkHandler = vkBuffer;
		graphicsBuffer->DeviceMemoryHandler = vkdeviceMemory;
		return graphicsBuffer;
	}

	VkImageView PuduGraphics::CreateImageView(ImageViewCreateData data)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = data.image;
		viewInfo.viewType = data.imageView;
		viewInfo.format = data.format;
		viewInfo.subresourceRange.aspectMask = data.aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = data.baseMipLevel;
		viewInfo.subresourceRange.levelCount = data.levelCount;
		viewInfo.subresourceRange.baseArrayLayer = data.baseArrayLayer;
		viewInfo.subresourceRange.layerCount = data.layerCount;

		VkImageView imageView;
		VKCheck(vkCreateImageView(m_device, &viewInfo, nullptr, &imageView), "failed to create texture image view!");

		if (data.name != nullptr)
		{
			SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)imageView, data.name);
		}

		return imageView;
	}

	void PuduGraphics::PickPhysicalDevice()
	{
		LOG("PickPhysicalDevice");
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			PUDU_ERROR("Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_physicalDevice = device;
				//For now just pick the first suitable device, later we can pick the most fancy one
				VkPhysicalDeviceProperties2 deviceProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
				VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
				VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
				indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

				VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES };

				VkPhysicalDeviceSynchronization2Features synchronization2Feature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES };
				synchronization2Feature.synchronization2 = VK_TRUE;

				void* currentPnext = &indexingFeatures;

				synchronization2Feature.pNext = currentPnext;
				currentPnext = &synchronization2Feature;

				timelineSemaphoreFeature.pNext = currentPnext;
				currentPnext = &timelineSemaphoreFeature;

				deviceFeatures.pNext = currentPnext;

				vkGetPhysicalDeviceProperties2(device, &deviceProperties);
				vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);

				Print("Picked device name %s", deviceProperties.properties.deviceName);
				Print("Device type %i", deviceProperties.properties.deviceType);

				break;
			}
		}

		if (m_physicalDevice == VK_NULL_HANDLE)
		{
			PUDU_ERROR("Failed to find a suitable GPU!");
		}
	}

	void PuduGraphics::CreateLogicalDevice()
	{
		LOG("CreateLogicalDevice");
		QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {
			indices.graphicsFamily.value(),
			indices.presentFamily.value(),
			indices.computeFamily.value(),
			indices.transferFamily.value() };

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;

		float queuePriority = 1.0f;

		for (auto queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceVulkan12Features featuresVulkan12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		featuresVulkan12.timelineSemaphore = VK_TRUE;
		featuresVulkan12.descriptorIndexing = VK_TRUE;
		featuresVulkan12.descriptorBindingPartiallyBound = VK_TRUE;
		featuresVulkan12.runtimeDescriptorArray = VK_TRUE;
		featuresVulkan12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		featuresVulkan12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
		featuresVulkan12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
		featuresVulkan12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		featuresVulkan12.separateDepthStencilLayouts = VK_TRUE;
		featuresVulkan12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;


		VkPhysicalDeviceFeatures2 deviceFeatures{};
		deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures.features.samplerAnisotropy = VK_TRUE;

		VkPhysicalDeviceSynchronization2Features syncFeatures{};
		syncFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
		syncFeatures.synchronization2 = VK_TRUE;

		VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES };
		dynamicRenderingFeatures.dynamicRendering = VK_TRUE;


		deviceFeatures.pNext = &syncFeatures;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		void* currentPNext = nullptr;

		syncFeatures.pNext = currentPNext;
		currentPNext = &syncFeatures;

		dynamicRenderingFeatures.pNext = currentPNext;
		currentPNext = &dynamicRenderingFeatures;

		featuresVulkan12.pNext = currentPNext;
		currentPNext = &featuresVulkan12;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

		VkPhysicalDevicePipelineCreationCacheControlFeatures cacheFeatures{};
		cacheFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES;
		cacheFeatures.pipelineCreationCacheControl = true;

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}
		if (m_physicalDeviceData.SupportsBindless)
		{

			/*m_physicalDeviceData.IndexingFeatures.pNext = currentPNext;

			currentPNext = &m_physicalDeviceData.IndexingFeatures;*/
		}

		deviceFeatures.pNext = currentPNext;
		currentPNext = &deviceFeatures;

		createInfo.pNext = currentPNext;

		if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}

		VkDeviceQueueInfo2 computeInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2 };
		computeInfo.queueFamilyIndex = indices.computeFamily.value();
		computeInfo.queueIndex = 0;

		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentationQueue);
		vkGetDeviceQueue2(m_device, &computeInfo, &m_computeQueue);
	}

	void PuduGraphics::CreateSurface()
	{
		if (glfwCreateWindowSurface(m_vkInstance, WindowPtr, m_allocatorPtr, &m_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface");
		}
	}

	void PuduGraphics::CreateSwapChain()
	{
		LOG("CreateSwapChain");

		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VKCheck(vkCreateSwapchainKHR(m_device, &createInfo, m_allocatorPtr, &m_swapChain), "failed to create swap chain!");

		m_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

		m_imageCount = imageCount;
		m_swapChainImageFormat = surfaceFormat.format;
		m_swapChainExtent = extent;


		for (uint32_t i = 0; i < m_swapChainImages.size(); i++)
		{
			SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE, (uint64_t)m_swapChainImages[i], fmt::format("Swapchain Image {}", i).c_str());
		}

		LOG("Swap chain images created!");
	}

	void PuduGraphics::CreateSwapchainImageViews()
	{
		LOG("Create Swapchain Images Views");
		m_swapChainImagesViews.resize(m_swapChainImages.size());

		for (size_t i = 0; i < m_swapChainImages.size(); i++)
		{
			ImageViewCreateData createData;
			createData.image = m_swapChainImages[i];
			createData.format = m_swapChainImageFormat;
			createData.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
			createData.name = fmt::format("Swapchain Image View {}", i).c_str();

			auto imageView = CreateImageView(createData);

			m_swapChainImagesViews[i] = imageView;

			//SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_swapChainImagesViews[i], );

			auto handle = m_resources.AllocateTexture2D();
			auto texture = m_resources.GetTexture<Texture2d>(handle->handle);

			texture->vkImageViewHandle = imageView;
			texture->vkImageHandle = m_swapChainImages[i];
			texture->format = m_swapChainImageFormat;
			texture->width = m_swapChainExtent.width;
			texture->height = m_swapChainExtent.height;
			texture->isSwapChain = true;

			m_swapChainTextures.push_back(texture);
		}

		LOG("Swapchain images created");
	}

	void PuduGraphics::SetResourceName(VkObjectType type, u64 handle, const char* name)
	{
		const VkDebugUtilsObjectNameInfoEXT resourceNameInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
			.pNext = NULL,
			.objectType = type,
			.objectHandle = (uint64_t)handle,
			.pObjectName = name,
		};

		pfnSetDebugUtilsObjectNameEXT(m_device, &resourceNameInfo);
	}

	void PuduGraphics::InitDebugUtilsObjectName()
	{
		pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(m_device, "vkSetDebugUtilsObjectNameEXT");

		if (pfnSetDebugUtilsObjectNameEXT == nullptr)
		{
			PUDU_ERROR("Debug utils object function not found");
		}
	}

	void PuduGraphics::CreateRenderPass(RenderPass* renderPass)
	{
		auto output = renderPass->attachments;

		VkAttachmentDescription2 colorAttachments[8] = {};
		VkAttachmentReference2 colorAttachmentsRef[8] = {};

		VkAttachmentLoadOp depthLoadOp, stencilLoadOp;
		VkImageLayout depthInitialLayout;

		switch (output.depthOperation)
		{
		case Load:
			depthLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthInitialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			break;
		case Clear:
			depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			break;
		default:
			depthLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			break;
		}

		switch (output.stencilOperation)
		{
		case Load:
			stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			break;
		case Clear:
			stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			break;
		default:
			stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			break;
		}

		//Color attachments
		uint32_t colorAttachmentsCount = 0;

		for (; colorAttachmentsCount < output.numColorFormats; colorAttachmentsCount++)
		{
			VkAttachmentLoadOp colorLoadOp;
			VkImageLayout colorInitialLayout;

			colorLoadOp = colorAttachments[colorAttachmentsCount].loadOp;
			colorInitialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription2& colorAttachment = colorAttachments[colorAttachmentsCount];
			colorAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			colorAttachment.format = output.colorAttachmentsFormat[colorAttachmentsCount];
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = colorLoadOp;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = stencilLoadOp;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = colorInitialLayout;
			colorAttachment.finalLayout = output.colorAttachments[colorAttachmentsCount].imageLayout;

			VkAttachmentReference2& colorAttachmentRef = colorAttachmentsRef[colorAttachmentsCount];
			colorAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			colorAttachmentRef.attachment = colorAttachmentsCount;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		//Depth attachment
		VkAttachmentDescription2 depthAttachment{};
		depthAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;

		VkAttachmentReference2 depthAttachmentRef{};
		depthAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;

		if (output.depthStencilFormat != VK_FORMAT_UNDEFINED)
		{
			depthAttachment.format = output.depthStencilFormat;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = depthLoadOp;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = stencilLoadOp;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = depthInitialLayout;
			depthAttachment.finalLayout = output.depthStencilFinalLayout;

			depthAttachmentRef.attachment = colorAttachmentsCount;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		//Create subpass
		VkSubpassDescription2 subpass{};
		subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkAttachmentDescription2 attachments[K_MAX_IMAGE_OUTPUTS + 1]{};
		for (uint32_t activeAttachmentIndex = 0; activeAttachmentIndex < output.numColorFormats; activeAttachmentIndex++)
		{
			attachments[activeAttachmentIndex] = colorAttachments[activeAttachmentIndex];
		}

		subpass.colorAttachmentCount = output.numColorFormats;
		subpass.pColorAttachments = colorAttachmentsRef;
		subpass.pDepthStencilAttachment = nullptr;

		uint32_t depthStencilCount = 0;
		if (output.depthStencilFormat != VK_FORMAT_UNDEFINED)
		{
			attachments[subpass.colorAttachmentCount] = depthAttachment;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			depthStencilCount++;
		}

		VkRenderPassCreateInfo2 renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
		renderPassInfo.attachmentCount = colorAttachmentsCount + depthStencilCount;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		vkCreateRenderPass2(m_device, &renderPassInfo, m_allocatorPtr, &renderPass->vkHandle);
	}

	ShaderStateHandle PuduGraphics::CreateShaderState(ShaderStateCreationData const& creation)
	{
		ShaderStateHandle handle = m_resources.AllocateShaderState();

		uint32_t compiledShaders = 0;

		ShaderState* shaderState = m_resources.GetShaderState(handle);
		shaderState->graphicsPipeline = true;
		shaderState->activeShaders = creation.stageCount;
		shaderState->name = creation.name;

		for (; compiledShaders < creation.stageCount; compiledShaders++)
		{
			auto stage = creation.stages[compiledShaders];

			VkPipelineShaderStageCreateInfo& shaderStageInfo = shaderState->shaderStageInfo[compiledShaders];
			shaderStageInfo = {};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = stage.type;
			shaderStageInfo.pName = SHADER_ENTRY_POINT;
			shaderStageInfo.module = CreateShaderModule(*stage.code, stage.codeSize, shaderState->name.c_str());
		}

		return handle;
	}

	void PuduGraphics::DestroyTexture(SPtr<Texture> texture)
	{
		if (!texture->IsDestroyed())
		{
			if (texture->isSwapChain)
			{
				return; //Swapchain textures are released by destroyswapchain
			}

			vkDestroyImage(m_device, texture->vkImageHandle, m_allocatorPtr);
			vkDestroyImageView(m_device, texture->vkImageViewHandle, m_allocatorPtr);
			vkDestroySampler(m_device, texture->Sampler.vkHandle, m_allocatorPtr);

			vkFreeMemory(m_device, texture->vkMemoryHandle, m_allocatorPtr);

			texture->Destroy();
		}
	}

	void PuduGraphics::CreateUniformBuffers() {
		m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkDeviceSize const bufferSize = sizeof(UniformBufferObject);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_uniformBuffers[i] = CreateGraphicsBuffer(bufferSize, nullptr, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "UniformBufer");

			vkMapMemory(m_device, m_uniformBuffers[i]->DeviceMemoryHandler, 0, bufferSize, 0, &m_uniformBuffers[i]->MappedMemory);
		}
	}


	void PuduGraphics::CreateLightingBuffers() {
		m_lightingBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkDeviceSize const bufferSize = sizeof(LightBuffer);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_lightingBuffers[i] = CreateGraphicsBuffer(bufferSize, nullptr, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "LightingBuffer");

			vkMapMemory(m_device, m_lightingBuffers[i]->DeviceMemoryHandler, 0, bufferSize, 0, &m_lightingBuffers[i]->MappedMemory);
		}
	}


	SPtr<Mesh> PuduGraphics::CreateMesh(MeshCreationData const& data)
	{
		auto vertices = data.Vertices;
		auto indices = data.Indices;

		SPtr<GraphicsBuffer> vertexBuffer = CreateGraphicsBuffer(sizeof(vertices[0]) * vertices.size(), vertices.data(),
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		SPtr<GraphicsBuffer> indexBuffer = CreateGraphicsBuffer(sizeof(indices[0]) * indices.size(), indices.data(),
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT);


		//TODO: Vertices and indices are being copied. Do we want that?
		auto mesh = m_resources.AllocateMesh();
		mesh->m_vertexBuffer = vertexBuffer;
		mesh->m_indexBuffer = indexBuffer;
		mesh->m_vertices = vertices;
		mesh->m_indices = indices;

		return mesh;
	}

	void PuduGraphics::CreateBindlessDescriptorPool()
	{
		LOG("Creating desciptor Pool");

		const uint32_t poolsSizesCount = 4;
		std::array< VkDescriptorPoolSize, poolsSizesCount> poolSizesBindless =
		{
			VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,k_MAX_BINDLESS_RESOURCES},
			VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, k_MAX_BINDLESS_RESOURCES},
			VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,k_MAX_BINDLESS_RESOURCES),
			VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,k_MAX_BINDLESS_RESOURCES),
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT; //support bindless
		poolInfo.maxSets = k_MAX_BINDLESS_RESOURCES * (uint32_t)poolSizesBindless.size();
		poolInfo.poolSizeCount = (uint32_t)poolSizesBindless.size();
		poolInfo.pPoolSizes = poolSizesBindless.data();

		m_physicalDeviceData.PoolSizesCount = poolsSizesCount;

		if (vkCreateDescriptorPool(m_device, &poolInfo, m_allocatorPtr, &m_bindlessDescriptorPool) != VK_SUCCESS)
		{
			PUDU_ERROR("Failed to create descriptor pool!");
		}
	}

	//A descriptor set layout is the template of the resources that are needed for a given render pipeline
	DescriptorSetLayoutHandle PuduGraphics::CreateBindlessDescriptorSetLayout(DescriptorSetLayoutData& creationData)
	{
		LOG("CreateDescriptorSetLayout");
		VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

		std::array< VkDescriptorBindingFlags, 4> bindingFlags;

		bindingFlags[0] = bindlessFlags;
		bindingFlags[1] = bindlessFlags;


		DescriptorSetLayoutHandle handle = m_resources.AllocateDescriptorSetLayout();

		for (auto& binding : creationData.Bindings) {
			binding.descriptorCount = k_MAX_BINDLESS_RESOURCES;
		}

		DescriptorSetLayout* descriptorSetLayout = m_resources.GetDescriptorSetLayout(handle);

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{};
		extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
		extendedInfo.bindingCount = (uint32_t)creationData.Bindings.size();
		extendedInfo.pBindingFlags = bindingFlags.data();

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = creationData.CreateInfo.bindingCount;
		createInfo.flags = creationData.CreateInfo.flags | VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		createInfo.pBindings = creationData.Bindings.data();
		createInfo.pNext = &extendedInfo;


		VkDescriptorSetLayout layout{};

		VKCheck(vkCreateDescriptorSetLayout(m_device, &createInfo, nullptr, &layout), "failed to create descriptor set layout!");

		descriptorSetLayout->vkHandle = layout;

		LOG("CreateDescriptorSetLayout End");

		return handle;
	}

	DescriptorSetLayoutHandle PuduGraphics::CreateDescriptorSetLayout(DescriptorSetLayoutData& data)
	{
		DescriptorSetLayoutHandle handle = m_resources.AllocateDescriptorSetLayout();

		for (auto& binding : data.Bindings) {
			binding.descriptorCount = 1;
		}

		DescriptorSetLayout* descriptorSetLayout = m_resources.GetDescriptorSetLayout(handle);

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = data.CreateInfo.bindingCount;
		createInfo.pBindings = data.Bindings.data();

		VkDescriptorSetLayout layout{};

		VKCheck(vkCreateDescriptorSetLayout(m_device, &createInfo, nullptr, &layout), "failed to create descriptor set layout!");

		descriptorSetLayout->vkHandle = layout;

		return handle;
	}

	void PuduGraphics::CreateDescriptorSets(VkDescriptorPool pool, VkDescriptorSet* descriptorSet, uint16_t setsCount, VkDescriptorSetLayout* layouts, uint32_t layoutsCount)
	{
		LOG("Creating descriptor set");

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = layoutsCount;
		allocInfo.pSetLayouts = layouts;
		allocInfo.descriptorSetCount = setsCount;

		VKCheck(vkAllocateDescriptorSets(m_device, &allocInfo, descriptorSet), "Failed creating descriptor set");

		LOG("Creating descriptor set end");
	}

	void PuduGraphics::DestroySemaphore(SPtr<Semaphore> semaphore)
	{
		vkDestroySemaphore(m_device, *semaphore, m_allocatorPtr);
	}

	/// <summary>
	/// In Vulkan NOn-Bindless resources shouldn't use a Bindless descriptor so we need to check what kind of descriptor we should create
	/// </summary>
	/// <param name="layoutData"></param>
	/// <param name="out"></param>
	void PuduGraphics::CreateDescriptorsLayouts(std::vector<DescriptorSetLayoutData>& layoutData, std::vector<DescriptorSetLayoutHandle>& out) {

		//Just have 1 set for now (bindless) so let's keep it simple
		for (auto& layoutData : layoutData)
		{
			if (layoutData.SetNumber == K_BINDLESS_SET_INDEX) //
			{
				out.emplace_back(CreateBindlessDescriptorSetLayout(layoutData));
			}
			else
			{
				out.emplace_back(CreateDescriptorSetLayout(layoutData));
			}
		}
	}


	PipelineHandle PuduGraphics::CreateGraphicsPipeline(PipelineCreationData& creationData)
	{
		LOG("CreateGraphicsPipeline Renderpass: {} Shader: {}", m_resources.GetRenderPass(creationData.renderPassHandle)->name.c_str(), creationData.shadersStateCreationData.name.c_str());

		PipelineHandle pipelineHandle = m_resources.AllocatePipeline();

		ShaderStateHandle shaderStateHandle = CreateShaderState(creationData.shadersStateCreationData);

		Pipeline* pipeline = m_resources.GetPipeline(pipelineHandle);
		pipeline->name.append(creationData.name);

		ShaderState* shaderState = m_resources.GetShaderState(shaderStateHandle);

		auto renderPass = m_resources.GetRenderPass(creationData.renderPassHandle);
		auto& renderPassOutput = renderPass->attachments;
		auto outputCount = renderPassOutput.colorAttachmentCount;

		pipeline->shaderState = shaderStateHandle;

		CreateDescriptorsLayouts(creationData.descriptorCreationData.layoutData, pipeline->descriptorSetLayoutHandles);

		for (uint32_t i = 0; i < pipeline->descriptorSetLayoutHandles.size(); i++)
		{
			pipeline->descriptorSetLayouts[i] = m_resources.GetDescriptorSetLayout(pipeline->descriptorSetLayoutHandles[i]);//DESCRIPTOR SET LAYOUTS NOT SET IN RESOURCES
		}
		pipeline->numActiveLayouts = pipeline->descriptorSetLayoutHandles.size();


		VkPushConstantRange pushConstant{};
		pushConstant.offset = 0;
		pushConstant.size = sizeof(UniformBufferObject);
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		//Push contants support
		VkPushConstantRange constants[1]{ pushConstant };

		std::vector<VkDescriptorSetLayout> pipelineDescriptorSetLayouts;
		for (uint32_t i = 0; i < pipeline->numActiveLayouts; i++)
		{
			pipelineDescriptorSetLayouts.push_back(pipeline->descriptorSetLayouts[i]->vkHandle);
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = pipelineDescriptorSetLayouts.size();
		pipelineLayoutInfo.pSetLayouts = pipelineDescriptorSetLayouts.data();
		pipelineLayoutInfo.pPushConstantRanges = constants;
		pipelineLayoutInfo.pushConstantRangeCount = 1;

		VkPipelineLayout pipelineLayout;
		vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, m_allocatorPtr, &pipelineLayout);

		pipeline->vkPipelineLayoutHandle = pipelineLayout;
		pipeline->numActiveLayouts = pipelineDescriptorSetLayouts.size();

		if (shaderState->graphicsPipeline)
		{
			VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
			graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

			graphicsPipelineInfo.pStages = shaderState->shaderStageInfo;
			graphicsPipelineInfo.stageCount = shaderState->activeShaders;

			graphicsPipelineInfo.layout = pipelineLayout;

			//Vertex input
			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			//Vertex attributes
			std::vector< VkVertexInputAttributeDescription> vertexAttributes(creationData.vertexInput.numVertexAttributes);

			uint32_t vertexAttribsCount = creationData.vertexInput.numVertexAttributes;

			for (size_t i = 0; i < vertexAttribsCount; i++)
			{
				VkVertexInputAttributeDescription& vertexAttribute = vertexAttributes[i];
				vertexAttribute = {};

				auto inputVertexAttrib = creationData.vertexInput.vertexAttributes[i];
				vertexAttribute.location = inputVertexAttrib.location;
				vertexAttribute.binding = inputVertexAttrib.binding;
				vertexAttribute.offset = inputVertexAttrib.offset;
				vertexAttribute.format = inputVertexAttrib.GetVkFormat();
			}

			vertexInputInfo.vertexAttributeDescriptionCount = vertexAttribsCount;
			vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();

			uint32_t vertexStreamsCount = creationData.vertexInput.numVertexStreams;


			//Vertex Bindings
			std::vector<VkVertexInputBindingDescription> vertexBindings(vertexStreamsCount);
			if (vertexStreamsCount)
			{
				for (size_t i = 0; i < vertexStreamsCount; i++)
				{
					VertexStream const& vertexStream = creationData.vertexInput.vertexStreams[i];
					VkVertexInputBindingDescription binding{};
					binding.binding = vertexStream.binding;
					binding.inputRate = vertexStream.GetVkInputRate();
					binding.stride = vertexStream.stride;

					vertexBindings[i] = binding;
				}

				vertexInputInfo.vertexBindingDescriptionCount = vertexBindings.size();
				vertexInputInfo.pVertexBindingDescriptions = vertexBindings.data();
			}

			graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;

			//Input assembly
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
			inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyInfo.topology = creationData.topology;
			inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

			graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyInfo;


			//Blending
			size_t blendCount = creationData.blendState.activeStatesCount;
			std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

			if (blendCount)
			{
				colorBlendAttachments.resize(blendCount);
				for (size_t i = 0; i < blendCount; i++)
				{
					auto& attachment = colorBlendAttachments[i];

					auto blendState = creationData.blendState.blendStates[i];

					attachment.blendEnable = blendState.blendEnabled;
					attachment.colorWriteMask = blendState.colorWriteMask;
					attachment.srcColorBlendFactor = blendState.sourceColorFactor;
					attachment.dstColorBlendFactor = blendState.destinationColorFactor;
					attachment.colorBlendOp = blendState.colorBlendOperation;
					attachment.srcAlphaBlendFactor = blendState.sourceAlphaFactor;
					attachment.dstAlphaBlendFactor = blendState.destinationAlphaFactor;
				}
			}
			else
			{
				colorBlendAttachments.resize(outputCount);
				for (u32 i = 0; i < outputCount; ++i) {
					colorBlendAttachments[i] = {};
					colorBlendAttachments[i].blendEnable = VK_FALSE;
					colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				}
			}

			VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
			colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendingInfo.logicOpEnable = VK_FALSE;
			colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
			colorBlendingInfo.attachmentCount = blendCount ? (uint32_t)blendCount : outputCount;
			colorBlendingInfo.pAttachments = colorBlendAttachments.data();

			graphicsPipelineInfo.pColorBlendState = &colorBlendingInfo;

			//Depth Stencil

			VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
			auto const& depthStencilState = creationData.depthStencil;

			depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilInfo.depthWriteEnable = depthStencilState.isDepthWriteEnable;
			depthStencilInfo.stencilTestEnable = false;
			depthStencilInfo.depthTestEnable = depthStencilState.isDepthEnabled;
			depthStencilInfo.depthCompareOp = depthStencilState.depthComparison;
			depthStencilInfo.front = depthStencilState.GetVkFront();
			depthStencilInfo.back = depthStencilState.GetVkBack();

			graphicsPipelineInfo.pDepthStencilState = &depthStencilInfo;

			//Multisample
			VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
			multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisamplingInfo.sampleShadingEnable = VK_FALSE;
			multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisamplingInfo.minSampleShading = 1.0f; // Optional
			multisamplingInfo.pSampleMask = nullptr; // Optional
			multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
			multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional

			graphicsPipelineInfo.pMultisampleState = &multisamplingInfo;

			//Rasterizer
			VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = creationData.rasterization.cullMode;
			rasterizer.frontFace = creationData.rasterization.front;
			rasterizer.depthBiasEnable = VK_TRUE; //TODO: This should be enabled only on shadowmapping pass
			//rasterizer.depthBiasConstantFactor = 0.0f; // Optional
			//rasterizer.depthClampEnable = VK_FALSE;
			//rasterizer.depthBiasClamp = 0.0f; // Optional
			//rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

			graphicsPipelineInfo.pRasterizationState = &rasterizer;

			//// Viewport state
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)m_swapChainExtent.width;
			viewport.height = (float)m_swapChainExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = { m_swapChainExtent.width, m_swapChainExtent.height };

			VkPipelineViewportStateCreateInfo viewportStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
			viewportStateInfo.viewportCount = 1;
			viewportStateInfo.pViewports = &viewport;
			viewportStateInfo.scissorCount = 1;
			viewportStateInfo.pScissors = &scissor;

			graphicsPipelineInfo.pViewportState = &viewportStateInfo;

			//RenderPass
			VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo{};
			pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
			pipelineRenderingInfo.colorAttachmentCount = renderPass->attachments.colorAttachmentCount;
			pipelineRenderingInfo.pColorAttachmentFormats = renderPass->attachments.colorAttachmentsFormat;
			pipelineRenderingInfo.depthAttachmentFormat = renderPass->attachments.depthStencilFormat;
			pipelineRenderingInfo.stencilAttachmentFormat = renderPass->attachments.GetStencilFormat();

			graphicsPipelineInfo.renderPass = nullptr;
			graphicsPipelineInfo.pNext = &pipelineRenderingInfo;
			// Dynamic states
			VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS };
			VkPipelineDynamicStateCreateInfo dynamic_state{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
			dynamic_state.dynamicStateCount = 3;
			dynamic_state.pDynamicStates = dynamicStates;

			graphicsPipelineInfo.pDynamicState = &dynamic_state;

			vkCreateGraphicsPipelines(m_device, nullptr, 1, &graphicsPipelineInfo, m_allocatorPtr, &pipeline->vkHandle);
			pipeline->vkPipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS; //TODO: ADD SUPPORT FOR COMPUTE
			pipeline->bindlessUpdated = false;
		}

		if (pipelineDescriptorSetLayouts.size() > 0)
		{
			//Create pipeline descriptor set, only handling bindless for now
			CreateDescriptorSets(m_bindlessDescriptorPool, pipeline->vkDescriptorSets, creationData.descriptorCreationData.setsCount, pipelineDescriptorSetLayouts.data(), 1);
			pipeline->numDescriptorSets = creationData.descriptorCreationData.setsCount;
		}

		LOG("Create Pipeline End");
		return pipelineHandle;
	}

	PipelineHandle PuduGraphics::CreateComputePipeline(ComputePipelineCreationData& creationData)
	{
		auto computeShader = m_resources.GetComputeShader(creationData.computeShaderHandle);
		VkPipelineShaderStageCreateInfo computeShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = computeShader->vkShaderModule;
		computeShaderStageInfo.pName = "main";

		auto pipelineHandle = m_resources.AllocatePipeline();
		Pipeline* pipeline = m_resources.GetPipeline(pipelineHandle);
		pipeline->vkPipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

		if (creationData.name != nullptr)
		{
			pipeline->name.append(creationData.name);
		}

		CreateDescriptorsLayouts(creationData.descriptorsCreationData.layoutData, pipeline->descriptorSetLayoutHandles);

		for (uint32_t i = 0; i < pipeline->descriptorSetLayoutHandles.size(); i++)
		{
			pipeline->descriptorSetLayouts[i] = m_resources.GetDescriptorSetLayout(pipeline->descriptorSetLayoutHandles[i]);//DESCRIPTOR SET LAYOUTS NOT SET IN RESOURCES
		}

		pipeline->numActiveLayouts = pipeline->descriptorSetLayoutHandles.size();

		std::vector<VkDescriptorSetLayout> pipelineDescriptorSetLayouts;
		for (uint32_t i = 0; i < pipeline->numActiveLayouts; i++)
		{
			pipelineDescriptorSetLayouts.push_back(pipeline->descriptorSetLayouts[i]->vkHandle);
		}


		VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		pipelineLayoutInfo.setLayoutCount = creationData.descriptorsCreationData.layoutData.size();
		pipelineLayoutInfo.pSetLayouts = pipelineDescriptorSetLayouts.data();


		VKCheck(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &pipeline->vkPipelineLayoutHandle), "Failed to create compute pipeline layout");

		VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
		pipelineInfo.layout = pipeline->vkPipelineLayoutHandle;
		pipelineInfo.stage = computeShaderStageInfo;

		if (pipelineDescriptorSetLayouts.size() > 0)
		{
			CreateDescriptorSets(m_bindlessDescriptorPool, pipeline->vkDescriptorSets, creationData.descriptorsCreationData.setsCount, pipelineDescriptorSetLayouts.data(), 1);
			pipeline->numDescriptorSets = creationData.descriptorsCreationData.setsCount;
		}

		pipeline->numDescriptorSets = creationData.descriptorsCreationData.setsCount;

		VKCheck(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline->vkHandle), "Failed to create compute pipeline");

		SetResourceName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipeline->vkHandle, pipeline->name.c_str());

		return pipelineHandle;
	}

	void PuduGraphics::UpdateComputeResources(ComputeShader* shader) {

		auto pipeline = m_resources.GetPipeline(shader->pipelineHandle);

		//PROXY TEX WRITE
		VkDescriptorImageInfo computeImage{};
		computeImage.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		computeImage.imageView = m_resources.GetTextureByName("forward_color")->vkImageViewHandle;

		VkWriteDescriptorSet drawImageWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		drawImageWrite.pNext = nullptr;

		drawImageWrite.dstBinding = 0;
		drawImageWrite.dstSet = pipeline->vkDescriptorSets[0];
		drawImageWrite.descriptorCount = 1;
		drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		drawImageWrite.pImageInfo = &computeImage;

		vkUpdateDescriptorSets(m_device, 1, &drawImageWrite, 0, nullptr);

		shader->MarkAsResourcesUpdated();
	}

	void PuduGraphics::CreateSwapChainFrameBuffers(RenderPassHandle renderPass)
	{

	}

	void PuduGraphics::CreateFrames()
	{
		m_Frames.resize(MAX_FRAMES_IN_FLIGHT);
	}

	void PuduGraphics::CreateCommandPool(VkCommandPool* cmdPool)
	{
		LOG("CreateCommandPool");
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		VKCheck(vkCreateCommandPool(m_device, &poolInfo, nullptr, cmdPool), "Failed to create command pool");
	}

	TextureHandle PuduGraphics::CreateTexture(TextureCreationData const& creationData)
	{
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.flags = 0;
		imageCreateInfo.imageType = ToVkImageType(creationData.textureType);
		imageCreateInfo.extent.width = creationData.width;
		imageCreateInfo.extent.height = creationData.height;
		imageCreateInfo.extent.depth = creationData.depth;
		imageCreateInfo.mipLevels = creationData.mipmaps;
		imageCreateInfo.arrayLayers = creationData.textureType == TextureType::Texture_Cube ? 6 : 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.format = creationData.format;

		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		const bool isRenderTarget = (creationData.flags & TextureFlags::RenderTargetMask) == TextureFlags::RenderTargetMask;
		const bool isComputeUsed = creationData.flags & TextureFlags::Compute;

		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageCreateInfo.usage |= isComputeUsed ? VK_IMAGE_USAGE_STORAGE_BIT : 0;

		if (TextureFormat::HasDepthOrStencil(creationData.format))
		{
			imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else
		{
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; //Read&write but it might but just for read
			imageCreateInfo.usage |= isRenderTarget ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
		}

		if (creationData.flags & TextureFlags::Sample)
		{
			imageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}

		if (creationData.textureType == TextureType::Texture_Cube)
		{
			imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

		VmaAllocationCreateInfo memoryInfo{};
		memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		SPtr<Texture> texture = nullptr;
		switch (creationData.textureType)
		{
		case TextureType::Texture2D:
			texture = m_resources.AllocateTexture2D();
			break;
		case TextureType::Texture_Cube:
			texture = m_resources.AllocateTextureCube();
			break;
		default:
			//PUDU_ERROR("Texture Type not supported {}", creationData.textureType);
			break;
		}

		texture->name.append(creationData.name);
		texture->width = creationData.width;
		texture->height = creationData.height;
		texture->format = creationData.format;
		texture->depth = creationData.depth;
		texture->layers = creationData.layers;
		texture->mipLevels = creationData.mipmaps;


		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = creationData.mipmaps;
		subresourceRange.layerCount = creationData.layers;

		uint32_t dataSize = creationData.dataSize;

		if (creationData.dataSize == -1)
		{
			dataSize = creationData.width * creationData.height * 4; //Assume RGBA8
		}

		texture->size = dataSize;

		//Allocate image
		vmaCreateImage(m_VmaAllocator, &imageCreateInfo, &memoryInfo, &texture->vkImageHandle, &texture->vmaAllocation, nullptr);

		//Allocate cubemap faces
		std::vector<VkBufferImageCopy2> bufferCopyRegions;
		if (creationData.textureType == TextureType::Texture_Cube)
		{
			subresourceRange.levelCount = creationData.mipmaps;
			subresourceRange.layerCount = 6;

			uint32_t offset = 0;

			for (uint32_t face = 0; face < 6; face++)
			{
				for (uint32_t layer = 0; layer < texture->layers; layer++)
				{
					for (size_t level = 0; level < creationData.mipmaps; level++)
					{
						ktx_size_t offset;
						KTX_error_code code = ktxTexture_GetImageOffset((ktxTexture*)creationData.sourceData, level, layer, face, &offset);
						assert(code == KTX_SUCCESS);
						VkBufferImageCopy2 bufferCopyRegion = { VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2 };
						bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						bufferCopyRegion.imageSubresource.mipLevel = level;
						bufferCopyRegion.imageSubresource.baseArrayLayer = face;
						bufferCopyRegion.imageSubresource.layerCount = 1;
						bufferCopyRegion.imageExtent.width = texture->width >> level;
						bufferCopyRegion.imageExtent.height = texture->height >> level;
						bufferCopyRegion.imageExtent.depth = 1;
						bufferCopyRegion.bufferOffset = offset;
						bufferCopyRegions.push_back(bufferCopyRegion);
					}
				}
			}
		}

		//Image view
		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = texture->vkImageHandle;
		imageViewInfo.viewType = ToVkImageViewType(creationData.textureType);
		imageViewInfo.format = imageCreateInfo.format;

		if (TextureFormat::HasDepthOrStencil(creationData.format))
		{
			subresourceRange.aspectMask = TextureFormat::HasDepth(creationData.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
		}

		imageViewInfo.subresourceRange = subresourceRange;

		vkCreateImageView(m_device, &imageViewInfo, m_allocatorPtr, &texture->vkImageViewHandle);

		SetResourceName(VK_OBJECT_TYPE_IMAGE, (uint64_t)texture->vkImageHandle, creationData.name);

		if (creationData.bindless)
		{
			UpdateBindlessTexture(texture->handle);
		}

		if (creationData.pixels != nullptr)
		{
			UploadTextureData(texture, creationData.pixels, subresourceRange, &bufferCopyRegions);
		}

		SamplerCreationData data = creationData.samplerData;
		data.maxLOD = texture->mipLevels;
		CreateTextureSampler(data, texture->Sampler.vkHandle);
		return texture->handle;
	}

	void PuduGraphics::UploadTextureData(SPtr<Texture> texture, void* pixels, VkImageSubresourceRange& range, std::vector<VkBufferImageCopy2>* regions)
	{
		//TODO: COMPUTE TEXTURE SIZE
		auto imageSize = texture->size;
		SPtr<GraphicsBuffer> stagingBuffer = CreateGraphicsBuffer(imageSize, nullptr, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VmaAllocationCreateInfo memoryInfo{};
		memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		void* data;
		vkMapMemory(m_device, stagingBuffer->DeviceMemoryHandler, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_device, stagingBuffer->DeviceMemoryHandler);

		auto cmd = BeginSingleTimeCommands();

		cmd.TransitionImageLayout(texture->vkImageHandle, texture->format, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &range);

		cmd.CopyBufferToImage(stagingBuffer->vkHandler, texture->vkImageHandle, static_cast<uint32_t>(texture->width),
			static_cast<uint32_t>(texture->height), regions);

		cmd.TransitionImageLayout(texture->vkImageHandle, texture->format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &range);

		EndSingleTimeCommands(cmd);
	}

	void PuduGraphics::CreateTextureImageView(Texture2d& texture2d)
	{
		ImageViewCreateData createData;
		createData.image = texture2d.vkImageHandle;
		createData.format = texture2d.format;
		createData.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

		texture2d.vkImageViewHandle = CreateImageView(createData);
	}

	void PuduGraphics::CreateTextureSampler(SamplerCreationData data, VkSampler& sampler)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		if (!data.wrap)
		{
			addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		}

		samplerInfo.addressModeU = addressMode;
		samplerInfo.addressModeV = addressMode;
		samplerInfo.addressModeW = addressMode;

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 1.0f;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = data.maxLOD;

		if (vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		{
			PUDU_ERROR("failed to create texture sampler!");
		}
	}

	void PuduGraphics::CreateFramesCommandBuffer()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)MAX_FRAMES_IN_FLIGHT * 2;

		std::vector<VkCommandBuffer> buffers;
		buffers.resize(MAX_FRAMES_IN_FLIGHT * 2); //Multiply by 2 for compute queue buffer

		if (vkAllocateCommandBuffers(m_device, &allocInfo, buffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffer!");
		}

		//We might need to fix the layout
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			Frame& frame = m_Frames[i];
			frame.CommandBuffer = GPUCommands(buffers[i * 2], this);
			frame.ComputeCommandBuffer = GPUCommands(buffers[i * 2 + 1], this);
		}

		LOG("Created command buffer");
	}

	void PuduGraphics::CreateSwapChainSyncObjects()
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_Frames[i].ImageAvailableSemaphore = CreateSemaphoreSPtr();
			m_Frames[i].RenderFinishedSemaphore = CreateSemaphoreSPtr();
			vkCreateFence(m_device, &fenceInfo, nullptr, &m_Frames[i].InFlightFence);
		}

		m_graphicsTimelineSemaphore = CreateTimelineSemaphore();
		m_computeTimelineSemaphore = CreateTimelineSemaphore();
	}

	void PuduGraphics::RecreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(WindowPtr, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwWaitEvents();
			glfwGetFramebufferSize(WindowPtr, &width, &height);

			if (glfwWindowShouldClose(WindowPtr))
			{
				return;
			}
		}

		vkDeviceWaitIdle(m_device);

		CleanupSwapChain();

		CreateSwapChain();
		CreateSwapchainImageViews();
		CreateDepthResources();

	}

	void PuduGraphics::UpdateUniformBuffer(uint32_t currentImage)
	{
		//NOT USED ANYMORE SINCE WE ARE PUSHING CONSTANTS FOR NOW
		/*UniformBufferObject ubo = GetUniformBufferObject();
		memcpy(m_uniformBuffers[currentImage].MappedMemory, &ubo, sizeof(ubo));*/
	}

	UniformBufferObject PuduGraphics::GetUniformBufferObject(Camera* cam, DrawCall& drawCall)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();

		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};

		ubo.modelMatrix = drawCall.TransformMatrix;
		ubo.viewMatrix = cam->GetViewMatrix();
		ubo.ProjectionMatrix = cam->Projection.GetProjectionMatrix();

		if (drawCall.GetRenderMaterial()->Shader->HasFragmentData())
		{
			if (drawCall.MaterialPtr.Texture != nullptr) //TODO: HERE WE SHOULD BIND ALL PRESENT TEXTURES
			{
				ubo.materialId = drawCall.MaterialPtr.Texture->handle.index;
			}
		}

		return ubo;
	}

	SPtr<Shader> PuduGraphics::CreateShader(fs::path fragmentPath, fs::path vertexPath, const char* name)
	{
		ShaderHandle handle = m_resources.AllocateShader();
		auto shader = m_resources.GetShader(handle);

		auto fragmentData = fragmentPath.empty() ? std::vector<char>() : FileManager::LoadShader(fragmentPath);
		auto vertexData = vertexPath.empty() ? std::vector<char>() : FileManager::LoadShader(vertexPath);

		shader->LoadFragmentData(fragmentData);
		shader->LoadVertexData(vertexData);
		shader->name = name;

		SPIRVParser::GetDescriptorSetLayout(shader.get(), shader->descriptors);

		return shader;
	}

	SPtr<ComputeShader> PuduGraphics::CreateComputeShader(fs::path shaderPath, const char* name)
	{
		LOG("Creating Compute Shader {}:", name);
		ComputeShaderHandle handle = m_resources.AllocateComputeShader();
		auto shader = m_resources.GetComputeShader(handle);

		auto data = FileManager::LoadShader(shaderPath);
		shader->vkShaderModule = CreateShaderModule(data, data.size() * sizeof(char), name);

		ComputePipelineCreationData creationData{};
		creationData.computeShaderHandle = handle;
		creationData.data = data;
		creationData.name = name;

		SPIRVParser::GetDescriptorSetLayout(data.data(), data.size() * sizeof(char), creationData.descriptorsCreationData);

		shader->pipelineHandle = CreateComputePipeline(creationData);

		return shader;
	}

	void PuduGraphics::UpdateBindlessResources(Pipeline* pipeline)
	{
		/*if (pipeline->bindlessUpdated)
		{
			return;
		}*/

		VkWriteDescriptorSet bindlessDescriptorWrites[k_MAX_BINDLESS_RESOURCES];
		VkDescriptorImageInfo bindlessImageInfos[k_MAX_BINDLESS_RESOURCES];
		uint32_t currentWriteIndex = 0;

		if (pipeline->numDescriptorSets != VK_FALSE)
		{
			for (int i = 0; i < m_bindlessResourcesToUpdate.size(); i++)
			{
				ResourceUpdate& textureToUpdate = m_bindlessResourcesToUpdate[i];

				auto texture = m_resources.GetTexture<Texture2d>({ textureToUpdate.handle });
				VkWriteDescriptorSet& descriptorWrite = bindlessDescriptorWrites[currentWriteIndex];
				descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.dstArrayElement = textureToUpdate.handle;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.dstSet = pipeline->vkDescriptorSets[0];

				descriptorWrite.dstBinding = k_BINDLESS_TEXTURE_BINDING;

				auto textureSampler = texture->Sampler;

				VkDescriptorImageInfo& descriptorImageInfo = bindlessImageInfos[currentWriteIndex];
				descriptorImageInfo.sampler = textureSampler.vkHandle;
				descriptorImageInfo.imageView = texture->vkImageViewHandle;
				descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				descriptorWrite.pImageInfo = &descriptorImageInfo;

				currentWriteIndex++;
			}
		}

		if (currentWriteIndex)
		{
			vkUpdateDescriptorSets(m_device, currentWriteIndex, bindlessDescriptorWrites, 0, nullptr);
		}

		pipeline->bindlessUpdated = true;
	}

	void PuduGraphics::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
		VkImageLayout newLayout)
	{
		auto commandBuffer = BeginSingleTimeCommands();

		commandBuffer.TransitionImageLayout(image, format, oldLayout, newLayout);

		EndSingleTimeCommands(commandBuffer);
	}

	SPtr<Texture2d> PuduGraphics::LoadTexture2D(fs::path filePath, TextureCreationSettings& settings)
	{
		return std::dynamic_pointer_cast<Texture2d>(LoadAndCreateTexture(filePath, settings));
	}

	SPtr<TextureCube> PuduGraphics::LoadTextureCube(fs::path filePath, TextureCreationSettings& settings)
	{
		return std::dynamic_pointer_cast<TextureCube>(LoadAndCreateTexture(filePath, settings));
	}


	void PuduGraphics::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, std::vector<VkBufferImageCopy2>* regions)
	{
		auto commandBuffer = BeginSingleTimeCommands();

		commandBuffer.CopyBufferToImage(buffer, image, width, height, regions);

		EndSingleTimeCommands(commandBuffer);
	}

	SPtr<Texture> PuduGraphics::LoadAndCreateTexture(fs::path path, TextureCreationSettings& settings)
	{
		LOG("Loading Texture {}", path.string());
		bool isKTX = path.extension() == ".ktx";
		void* pixelsData = nullptr;
		void* sourceData = nullptr;

		int texWidth, texHeight, texChannels = 0;
		int depth = 1;
		uint32_t dataSize = -1;
		uint32_t layers = 1;
		uint32_t levels = 1;

		TextureType::Enum textureType = settings.textureType;

		ktxTexture* ktxTexture;
		if (isKTX)
		{
			auto result = ktxTexture_CreateFromNamedFile(FileManager::GetAssetPath(path).string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

			sourceData = ktxTexture;

			pixelsData = ktxTexture->pData;

			texWidth = ktxTexture->baseWidth;
			texHeight = ktxTexture->baseHeight;
			depth = ktxTexture->baseDepth;
			layers = ktxTexture->numLayers;
			levels = ktxTexture->numLevels;

			dataSize = ktxTexture_GetSize(ktxTexture);

			if (ktxTexture->isCubemap)
			{
				textureType = TextureType::Texture_Cube;
			}
		}
		else
		{
			pixelsData = stbi_load(FileManager::GetAssetPath(path).string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

			dataSize = texWidth * texHeight * 4; //TODO: Assume 32bit Compute with mipmap
		}

		TextureCreationData creationData;
		creationData.bindless = settings.bindless;
		creationData.depth = depth;
		creationData.width = texWidth;
		creationData.height = texHeight;
		creationData.name = settings.name;
		creationData.format = settings.format;
		creationData.mipmaps = levels;
		creationData.pixels = pixelsData;
		creationData.flags = TextureFlags::Sample;
		creationData.textureType = textureType;
		creationData.dataSize = dataSize;
		creationData.layers = layers;
		creationData.sourceData = sourceData;

		auto textureHandle = CreateTexture(creationData);

		if (isKTX)
		{
			ktxTexture_Destroy(ktxTexture);
		}
		else {
			stbi_image_free(pixelsData);
		}

		return m_resources.GetTexture<Texture>(textureHandle);
	}

	void PuduGraphics::DestroyRenderPass(SPtr<RenderPass> handle)
	{
		vkDestroyRenderPass(m_device, handle->vkHandle, nullptr);
	}

	void PuduGraphics::DestroyFrameBuffer(Framebuffer& handle)
	{
		vkDestroyFramebuffer(m_device, handle.vkHandle, nullptr);
	}

	Model PuduGraphics::CreateModel(std::shared_ptr<Mesh> mesh, Material& material)
	{
		Model model;

		std::vector<std::shared_ptr<Mesh>> meshes{ mesh };
		std::vector<Material> materials{ material };
		model.Meshes = meshes;
		model.Materials = materials;

		return model;
	}

	Model PuduGraphics::CreateModel(MeshCreationData const& data)
	{
		auto mesh = CreateMesh(data);

		Material material = Material();
		const std::filesystem::path path = data.Material.BaseTexturePath;
		if (data.Material.hasBaseTexture)
		{

			TextureCreationSettings settings{};
			settings.bindless = true;
			settings.name = "Albedo";

			material.Texture = LoadTexture2D(path, settings);
		}

		if (data.Material.hasNormalMap)
		{
			int texWidth, texHeight, texChannels;

			TextureCreationSettings normalCreation;
			normalCreation.bindless = true;
			normalCreation.name = "Normal";
			normalCreation.format = VK_FORMAT_R8G8B8A8_UNORM;

			material.NormalMap = LoadTexture2D(path, normalCreation);
		}

		auto m = CreateModel(mesh, material);
		m.Name = data.Name;
		return m;
	}

	static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

	void PuduGraphics::InitImgui()
	{
		LOG("ImGUI Init");

		ImGui::CreateContext();
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
		ImGui_ImplGlfw_InitForVulkan(WindowPtr, true);

		// Create Descriptor Pool
		// The example only requires a single combined image sampler descriptor for the font image and only uses one descriptor set (for that)
		// If you wish to load e.g. additional textures you may need to alter pools sizes.
		{
			{
				VkDescriptorPoolSize pool_sizes[] =
				{
					{VK_DESCRIPTOR_TYPE_SAMPLER, 1},
					{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
					{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1},
					{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
					{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1},
					{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1},
					{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
					{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
					{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1},
					{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1},
					{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1}
				};

				VkDescriptorPoolCreateInfo pool_info = {};
				pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
				pool_info.maxSets = IM_ARRAYSIZE(pool_sizes);
				pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
				pool_info.pPoolSizes = pool_sizes;

				if (vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_ImGuiDescriptorPool) != VK_SUCCESS)
					throw std::runtime_error("Create DescriptorPool for m_ImGuiDescriptorPool failed!");
			}
		}

		CreateImGuiRenderPass();
		CreateCommandPool(&m_ImGuiCommandPool);
		CreateImGUIFrameBuffers();
		CreateImGUICommandBuffers();

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = m_vkInstance;
		initInfo.PhysicalDevice = m_physicalDevice;
		initInfo.Device = m_device;
		initInfo.QueueFamily = FindQueueFamilies(m_physicalDevice).graphicsFamily.value();
		initInfo.Queue = m_graphicsQueue;
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.DescriptorPool = m_ImGuiDescriptorPool;
		initInfo.Subpass = 0;
		initInfo.ImageCount = m_imageCount;
		initInfo.MinImageCount = m_imageCount;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.Allocator = m_allocatorPtr;
		initInfo.CheckVkResultFn = check_vk_result;

		//initInfo.PipelineCache = m_pipelineCache;

		ImGui_ImplVulkan_Init(&initInfo, m_ImGuiRenderPass);
		vkDeviceWaitIdle(m_device);

		//ImGui_ImplVulkan_DestroyFontsTexture();
		LOG("ImGUI init end");
	}

	void PuduGraphics::CreateDepthResources()
	{
		//LOG("CreateDepthResources");
		//VkFormat depthFormat = FindDepthFormat();

		//ImageCreateData createImageData;
		//createImageData.width = m_swapChainExtent.width;
		//createImageData.height = m_swapChainExtent.height;
		//createImageData.format = depthFormat;
		//createImageData.tilling = VK_IMAGE_TILING_OPTIMAL;
		//createImageData.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		//createImageData.memoryPropertiesFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		//createImageData.image = &m_depthImage;
		//createImageData.imageMemory = &m_depthImageMemory;


		//CreateImage(createImageData);

		//ImageViewCreateData imageViewData;
		//imageViewData.image = m_depthImage;
		//imageViewData.format = depthFormat;
		//imageViewData.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

		//m_depthImageView = CreateImageView(imageViewData);

		//TransitionImageLayout(m_depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		//	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}



	VkFormat PuduGraphics::FindDepthFormat()
	{
		return FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}



	VkFormat PuduGraphics::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
		VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		PUDU_ERROR("Failed to find supported format!");
	}

	GPUCommands PuduGraphics::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return GPUCommands(commandBuffer, this);
	}

	SPtr<Semaphore> PuduGraphics::CreateSemaphoreSPtr()
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		auto semaphore = m_resources.AllocateSemaphore();

		vkCreateSemaphore(m_device, &semaphoreInfo, m_allocatorPtr, *semaphore);

		return semaphore;
	}

	SPtr<Semaphore> PuduGraphics::CreateTimelineSemaphore()
	{
		VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkSemaphoreTypeCreateInfo semaphoreTypeInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
		semaphoreTypeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

		semaphoreInfo.pNext = &semaphoreTypeInfo;

		auto semaphore = m_resources.AllocateSemaphore();

		vkCreateSemaphore(m_device, &semaphoreInfo, m_allocatorPtr, &semaphore->vkHandle);

		return semaphore;
	}

	void PuduGraphics::EndSingleTimeCommands(GPUCommands commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer.vkHandle);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer.vkHandle;

		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue);

		vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer.vkHandle);
	}

	VkShaderModule PuduGraphics::CreateShaderModule(const std::vector<char>& code, size_t size, const char* name)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = size;
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module!");
		}

		if (name != nullptr)
		{
			SetResourceName(VK_OBJECT_TYPE_SHADER_MODULE, (uint64_t)shaderModule, name);
		}

		return shaderModule;
	}

	SwapChainSupportDetails PuduGraphics::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

		if (formatCount > 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

		if (presentModeCount > 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount,
				details.presentModes.data());
		}

		return details;
	}

	bool PuduGraphics::IsDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties2 deviceProperties{};
		deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;


		void* currentPNext = nullptr;

		VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
		indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

		currentPNext = &indexingFeatures;

		//This feature is to simplify barriers and semaphore
		VkPhysicalDeviceSynchronization2Features synchronization2Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES };
		synchronization2Features.synchronization2 = VK_TRUE;

		VkPhysicalDeviceTimelineSemaphoreFeatures semaphoreFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES };


		synchronization2Features.pNext = currentPNext;
		currentPNext = &synchronization2Features;
		semaphoreFeatures.pNext = currentPNext;
		currentPNext = &semaphoreFeatures;


		VkPhysicalDeviceFeatures2 deviceFeatures{};
		deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures.pNext = currentPNext;

		vkGetPhysicalDeviceProperties2(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);

		QueueFamilyIndices indices = FindQueueFamilies(device);

		bool bindlessSupported =
			indexingFeatures.descriptorBindingPartiallyBound &&
			indexingFeatures.runtimeDescriptorArray;

		if (deviceProperties.properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			return false; //Force not using integrated for now
		}

		bool extensionsSupported = CheckDeviceExtensionSupport(device);
		bool swapChainAdequate = false;

		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		// These are required to support the 4 descriptor binding flags we use in this sample.
		indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
		// Enables use of runtimeDescriptorArrays in SPIR-V shaders.
		indexingFeatures.runtimeDescriptorArray = VK_TRUE;

		LOG("Bindless suported {}", bindlessSupported);
		m_physicalDeviceData.IndexingFeatures = indexingFeatures;
		m_physicalDeviceData.SupportsBindless = bindlessSupported;

		return indices.IsComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.features.samplerAnisotropy && bindlessSupported;
	}

	bool PuduGraphics::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices PuduGraphics::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (queueFamily.queueFlags * VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}
			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.computeFamily = i;
			}
			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				indices.transferFamily = i;
			}

			if (indices.IsComplete())
				break;

			i++;
		}

		return indices;
	}

	void PuduGraphics::Cleanup()
	{
		if (!m_initialized)
		{
			return;
		}
		CleanupSwapChain();

		m_resources.DestroyAllResources(this);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();


		vkDestroyDescriptorPool(m_device, m_bindlessDescriptorPool, m_allocatorPtr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyFence(m_device, m_Frames[i].InFlightFence, m_allocatorPtr);
		}

		vkDestroyCommandPool(m_device, m_commandPool, m_allocatorPtr);

		//ImGui
		{
			vkDestroyCommandPool(m_device, m_ImGuiCommandPool, m_allocatorPtr);
			vkDestroyRenderPass(m_device, m_ImGuiRenderPass, m_allocatorPtr);
			vkDestroyDescriptorPool(m_device, m_ImGuiDescriptorPool, m_allocatorPtr);
		}

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
		}


		vkDestroySurfaceKHR(m_vkInstance, m_surface, m_allocatorPtr); //Be sure to destroy surface before instance

		vkDestroyDevice(m_device, m_allocatorPtr);

		vkDestroyInstance(m_vkInstance, nullptr);

		glfwDestroyWindow(WindowPtr);

		glfwTerminate();

		m_initialized = false;
	}

	void PuduGraphics::CleanupSwapChain()
	{
		vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	}

	void PuduGraphics::DestroyMesh(SPtr<Mesh> mesh)
	{
		if (!mesh->IsDisposed())
		{
			DestroyBuffer(mesh->GetIndexBuffer());
			DestroyBuffer(mesh->GetVertexBuffer());
			mesh->Destroy();
		}
	}

	void PuduGraphics::WaitIdle()
	{
		vkDeviceWaitIdle(m_device);
	}
}

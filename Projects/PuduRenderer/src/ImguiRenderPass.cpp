#include "ImguiRenderPass.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_vulkan.h"

#include "ImGuiUtils.h"
#include "Logger.h"
#include "PuduGraphics.h"

namespace Pudu
{
	void ImguiRenderPass::Initialize(PuduGraphics* gpu)
	{
		LOG("ImGUI Init");

		ImGui::CreateContext();
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan(gpu->WindowPtr, true);

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

		ImGui_ImplVulkan_Init(&initInfo, m_ImGuiRenderPass->vkHandle);
		vkDeviceWaitIdle(m_device);

		//ImGui_ImplVulkan_DestroyFontsTexture();
		LOG("ImGUI init end");
	}
}

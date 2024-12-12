#include "ImguiRenderPass.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_vulkan.h"

#include "ImGuiUtils.h"
#include "Logger.h"
#include "PuduGraphics.h"


namespace Pudu
{
	void ImguiRenderPass::Render(RenderFrameData& frameData)
	{
		auto imguiCommands = frameData.currentCommand;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		ImGui::Begin("Pudu Renderer Debug");

		frameData.app->DrawImGUI();

		ImGui::End();
		ImGui::Render();

		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiCommands->vkHandle);

		ImGui::EndFrame();
	}
	void ImguiRenderPass::Initialize(PuduGraphics* gpu)
	{
		LOG("ImGUI Init");

		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan(gpu->WindowPtr, true);


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

		DescriptorPoolCreationData descriptorPoolData;

		descriptorPoolData
			.AddDescriptorType(VK_DESCRIPTOR_TYPE_SAMPLER, 1)
			->AddDescriptorType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			->AddDescriptorType(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1)
			->AddDescriptorType(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
			->AddDescriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1)
			->AddDescriptorType(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1)
			->AddDescriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
			->AddDescriptorType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1)
			->AddDescriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1)
			->AddDescriptorType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1)
			->AddDescriptorType(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1);


		descriptorPoolData.bindless = false;
		m_descriptorPool = gpu->GetDescriptorPool(descriptorPoolData);
		m_ImGuiCommandPool = gpu->GetCommandPool(QueueFamily::Graphics);

		m_ImGuiCommandBuffers = gpu->CreateCommandBuffers({ .pool = m_ImGuiCommandPool->vkHandle, .count = gpu->GetImageCount() });

		//CreateImguiFrameBuffers(gpu);

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = gpu->GetVkInstance();
		initInfo.PhysicalDevice = gpu->GetPhysicalDevice();
		initInfo.Device = gpu->GetDevice();
		initInfo.QueueFamily = gpu->GetQueueFamiliesIndex().graphicsFamily.value();
		initInfo.Queue = gpu->GetGraphicsQueue();
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.DescriptorPool = m_descriptorPool->vkHandle;
		initInfo.Subpass = 0;
		initInfo.ImageCount = gpu->GetImageCount();
		initInfo.MinImageCount = gpu->GetImageCount();
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.Allocator = nullptr;
		initInfo.CheckVkResultFn = nullptr;
		initInfo.UseDynamicRendering = true;
		initInfo.ColorAttachmentFormat = attachments.colorAttachments[0].resource->format;

		ImGui_ImplVulkan_Init(&initInfo, vkHandle);
		vkDeviceWaitIdle(gpu->GetDevice());

		LOG("ImGUI init end");
	}
	void ImguiRenderPass::CreateImguiFrameBuffers(PuduGraphics* gpu)
	{
		LOG("Create ImGUI FrameBuffers");

		m_ImGuiFrameBuffers.resize(gpu->GetImageCount());

		for (size_t i = 0; i < gpu->GetImageCount(); i++)
		{
			std::array<VkImageView, 1> attachments = {
				gpu->GetSwapchainImageViews()->at(i)
			};

			FramebufferCreationData fbCreationData;
			fbCreationData.AddRenderTexture(gpu->GetSwapchainTextures()->at(i)->Handle());
			fbCreationData.renderPassHandle = this->Handle();
			fbCreationData.width = gpu->GetSwapchainExtend().width;
			fbCreationData.height = gpu->GetSwapchainExtend().height;

			m_ImGuiFrameBuffers[i] = gpu->CreateFramebuffer(fbCreationData);
		}
	}
}

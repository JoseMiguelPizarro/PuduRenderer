#pragma once
#include "FrameGraph/RenderPass.h"
#include "Resources/CommandPool.h"
#include "Resources/DescriptorPool.h"
#include "RenderFrameData.h"

namespace Pudu
{
	class ImguiRenderPass :public RenderPass
	{
	public:
		SPtr<GPUCommands> commands;
		SPtr<CommandPool> m_ImGuiCommandPool;
		SPtr<DescriptorPool> m_descriptorPool;
		std::vector<SPtr<GPUCommands>> m_ImGuiCommandBuffers;
		std::vector<SPtr<Framebuffer>> m_ImGuiFrameBuffers;
		void Render(RenderFrameData& frameData) override;

	private:
		void Initialize(PuduGraphics* gpu) override;
		void CreateImguiFrameBuffers(PuduGraphics* gpu);
	};
}
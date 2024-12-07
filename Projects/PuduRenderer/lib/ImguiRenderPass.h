#pragma once
#include "FrameGraph/RenderPass.h"
#include "Resources/CommandPool.h"

namespace Pudu
{
	class ImguiRenderPass :public RenderPass
	{
	public:
		SPtr<GPUCommands> commands;
		SPtr<CommandPool> m_ImGuiCommandPool;
		std::vector<SPtr<GPUCommands>> m_ImGuiCommandBuffers;
		std::vector<SPtr<Framebuffer>> m_ImGuiFrameBuffers;

	private:
		void Initialize(PuduGraphics* gpu) override;
	};
}
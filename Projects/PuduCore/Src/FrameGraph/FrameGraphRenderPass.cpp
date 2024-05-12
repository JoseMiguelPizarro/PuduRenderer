#include "FrameGraph/FrameGraphRenderPass.h"
#include "RenderFrameData.h"
#include "DrawCall.h"
#include "PuduRenderer.h"
#include "Resources/Resources.h"

namespace Pudu
{
	Pipeline* FrameGraphRenderPass::GetPipeline(RenderFrameData& frameData, DrawCall& drawcall)
	{
		return frameData.renderer->GetPipeline(frameData, RenderPassType::Color);
	}

	void FrameGraphRenderPass::Render(RenderFrameData& frameData)
	{
		auto renderScene = frameData.scene;
		auto commands = frameData.currentCommand;

		for (DrawCall drawCall : renderScene->GetDrawCalls()) {

			Model model = drawCall.ModelPtr;
			auto mesh = drawCall.MeshPtr;

			frameData.currentDrawCall = &drawCall;

			VkBuffer vertexBuffers[] = { mesh->GetVertexBuffer()->Handler };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commands->vkHandle, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commands->vkHandle, mesh->GetIndexBuffer()->Handler, 0, VK_INDEX_TYPE_UINT32);

			auto ubo = frameData.graphics->GetUniformBufferObject(*renderScene->camera, drawCall);
			uint32_t materialid = drawCall.MaterialPtr.Texture->handle.index;

			Pipeline* pipeline = GetPipeline(frameData, drawCall);
			frameData.graphics->UpdateBindlessResources(pipeline);

			Viewport viewport;
			viewport.rect = { 0,0, (uint16)frameData.graphics->WindowWidth,(uint16)frameData.graphics->WindowHeight };
			viewport.maxDepth = 1;
			commands->SetViewport(viewport);
			commands->BindPipeline(pipeline);
			vkCmdBindDescriptorSets(commands->vkHandle, pipeline->vkPipelineBindPoint, pipeline->vkPipelineLayoutHandle, 0, pipeline->numActiveLayouts, &pipeline->vkDescriptorSet, 0, nullptr);
			vkCmdPushConstants(commands->vkHandle, pipeline->vkPipelineLayoutHandle, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UniformBufferObject), &ubo);
			vkCmdPushConstants(commands->vkHandle, pipeline->vkPipelineLayoutHandle, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(UniformBufferObject), sizeof(uint32_t), &materialid);

			vkCmdDrawIndexed(commands->vkHandle, static_cast<uint32_t>(mesh->GetIndices()->size()), 1, 0, 0, 0);
		}
	}
}

#include "FileManager.h"
#include "PostProcessingRenderPass.h"
#include <filesystem>

namespace Pudu
{
	void PostProcessingRenderPass::Initialize(PuduGraphics* gpu)
	{
		MeshCreationData meshCreation = {
			.Name = "quad",
		.Vertices = quadVertices,
		.Indices = quadIndices,
		};

		m_quadMesh = gpu->CreateMesh(meshCreation);
		auto fragmentshaderPath = FileManager::GetAssetPath(m_shaderPath);
		auto vertexshaderPath = FileManager::GetAssetPath(m_shaderPathV);

		m_screenColor = gpu->GetRenderTexture();
		m_screenColor->bindless = true;
		m_screenColor->width = gpu->WindowWidth;
		m_screenColor->height = gpu->WindowHeight;
		m_screenColor->depth = 1;
		m_screenColor->format = gpu->GetSurfaceFormat();
		m_screenColor->Create(gpu);

		m_postProcessingShader = gpu->CreateShader(fragmentshaderPath, vertexshaderPath, "Postprocessing");
	}

	void PostProcessingRenderPass::Render(RenderFrameData& renderData)
	{
		auto command = renderData.currentCommand;

		command->BindMesh(m_quadMesh.get());
		Pipeline* pipeline = GetPipeline({
			.renderPass = this,
			.shader = m_postProcessingShader.get(),
			.renderer = renderData.renderer,
			});

		command->BindPipeline(pipeline);

		command->DrawIndexed(m_quadMesh->GetIndices()->size());
	}

	void PostProcessingRenderPass::PreRender(RenderFrameData& renderData)
	{
		auto command = renderData.currentCommand;
		auto frameColor = attachments.colorAttachments[0].resource;


		command->TransitionImageLayout(frameColor->vkImageHandle, frameColor->format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		command->Blit(attachments.colorAttachments[0].resource, m_screenColor, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		command->TransitionImageLayout(m_screenColor->vkImageHandle, m_screenColor->format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		command->TransitionImageLayout(frameColor->vkImageHandle, m_screenColor->format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}

}
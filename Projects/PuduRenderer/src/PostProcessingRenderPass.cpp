#include "FileManager.h"
#include "PostProcessingRenderPass.h"
#include <filesystem>

namespace Pudu
{
	void PostProcessingRenderPass::Initialize(PuduGraphics* gpu)
	{
		m_material = gpu->Resources()->AllocateMaterial();

		MeshCreationData meshCreation = {
			.Name = "quad",
		.Vertices = quadVertices,
		.Indices = quadIndices,
		};

		m_quadMesh = gpu->CreateMesh(meshCreation);

		m_screenColor = gpu->GetRenderTexture();
		m_screenColor->name = "ScreenColor";
		m_screenColor->bindless = true;
		m_screenColor->width = gpu->WindowWidth;
		m_screenColor->height = gpu->WindowHeight;
		m_screenColor->depth = 1;
		m_screenColor->format = gpu->GetSurfaceFormat();
		m_screenColor->SetImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		m_screenColor->Create(gpu);

		m_postProcessingShader = gpu->CreateShader(m_shaderPath, "Postprocessing");
		m_material->SetShader(m_postProcessingShader);
		m_material->SetProperty("Data.colorTex", m_screenColor);
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

		m_material->ApplyProperties();
		command->BindPipeline(pipeline);

		BindMaterialDescriptorSets(pipeline,m_material,renderData);

		command->DrawIndexed(m_quadMesh->GetIndices()->size());
	}
	void PostProcessingRenderPass::PreRender(RenderFrameData& renderData)
	{
		auto command = renderData.currentCommand;
		auto frameColor = attachments.colorAttachments[0].resource;

		command->TransitionTextureLayout(m_screenColor, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		command->TransitionTextureLayout(frameColor, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		command->Blit(attachments.colorAttachments[0].resource, m_screenColor, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		command->TransitionTextureLayout(m_screenColor, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		command->TransitionTextureLayout(frameColor, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
}
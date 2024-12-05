
#include "FrameGraph/PostProcessingRenderPass.h"
#include <filesystem>
//#include "FileManager.h"

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
	/*	auto fragmentshaderPath = FileManager::GetAssetPath(m_shaderPath);
		auto vertexshaderPath = FileManager::GetAssetPath(m_shaderPathV);

		m_postProcessingShader = gpu->CreateShader(fragmentshaderPath, vertexshaderPath, "Postprocessing");*/
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

}
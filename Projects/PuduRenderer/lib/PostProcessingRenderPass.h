#pragma once

#include "FrameGraph/RenderPass.h"
#include "vertex.h"
#include "PuduGraphics.h"
#include "RenderFrameData.h"


namespace Pudu
{
	const static  std::vector<Vertex> quadVertices = {
		{.pos = {-.5f,-.5f,0}, .texcoord = {0.f,0.f}},
		{.pos = {-.5f,.5f,0.f}, .texcoord = {0.f,1.f}},
		{.pos = {.5f,0.5f,0.f},.texcoord = {1.f,1.f}},
		{.pos = {.5f,-0.5f,0.f},.texcoord = {1.f,0.f}} };

	const static std::vector<uint32_t> quadIndices = {
		0,1,2,
		0,2,3
	};

	class PostProcessingRenderPass :public RenderPass
	{
	public:
		void Initialize(PuduGraphics* gpu) override;
		void Render(RenderFrameData& renderData) override;
		void PreRender(RenderFrameData& renderData) override;

	private:
		SPtr<Mesh> m_quadMesh;
		SPtr<Shader> m_postProcessingShader;
		SPtr<RenderTexture> m_screenColor;

		std::filesystem::path m_shaderPath = "Shaders/postprocessing.frag";
		std::filesystem::path m_shaderPathV = "Shaders/postprocessing.vert";
	};
}
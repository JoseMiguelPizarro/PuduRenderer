#include <unordered_map>
#include <vector>
#include "PuduRenderer.h"
#include "SPIRVParser.h"
#include "ForwardRenderPass.h"
#include "DepthStencilRenderPass.h"
#include "PostProcessingRenderPass.h"
#include "ComputeRenderPass.h"
#include <Logger.h>
#include "Shader.h"

namespace Pudu
{
	void PuduRenderer::OnInit(PuduGraphics* graphics, PuduApp* app)
	{
		this->graphics = graphics;
		this->app = app;

		auto depthRT = graphics->GetRenderTexture();
		depthRT->depth = 1;
		depthRT->width = graphics->WindowWidth;
		depthRT->height = graphics->WindowHeight;
		depthRT->format = VK_FORMAT_D32_SFLOAT;
		depthRT->name = "DepthPrepassTexture";

		auto shadowRT = graphics->GetRenderTexture();
		shadowRT->depth = 1;
		shadowRT->width = graphics->WindowWidth;
		shadowRT->height = graphics->WindowHeight;
		shadowRT->format = VK_FORMAT_D16_UNORM;
		shadowRT->name = "ShadowMap";

		auto colorRT = graphics->GetRenderTexture();
		colorRT->depth = 1;
		colorRT->width = graphics->WindowWidth;
		colorRT->height = graphics->WindowHeight;
		colorRT->format = VK_FORMAT_R8G8B8A8_UNORM;
		colorRT->name = "ForwardColor";

		m_depthRenderPass = graphics->GetRenderPass<DepthPrepassRenderPass>();
		m_depthRenderPass->name = "DepthPrepassRenderPass";
		m_depthRenderPass->AddDepthStencilAttachment(depthRT, AttachmentUsage::Write, LoadOperation::Clear);

		m_shadowMapRenderPass = graphics->GetRenderPass<ShadowMapRenderPass>();
		m_shadowMapRenderPass->name = "ShadowMapRenderPass";
		m_shadowMapRenderPass->AddDepthStencilAttachment(shadowRT, AttachmentUsage::Write, LoadOperation::Clear);

		m_forwardRenderPass = graphics->GetRenderPass<ForwardRenderPass>();
		m_forwardRenderPass->name = "ForwardRenderPass";
		m_forwardRenderPass->AddColorAttachment(colorRT, AttachmentUsage::Write, LoadOperation::Clear);
		m_forwardRenderPass->AddColorAttachment(shadowRT, AttachmentUsage::Read, LoadOperation::Load);
		m_forwardRenderPass->AddDepthStencilAttachment(depthRT, AttachmentUsage::Read, LoadOperation::Load);

		m_postProcessingRenderPass = graphics->GetRenderPass<PostProcessingRenderPass>();
		m_postProcessingRenderPass->name = "Postprocessing";
		m_postProcessingRenderPass->AddColorAttachment(colorRT, AttachmentUsage::Write, LoadOperation::Load);

		m_imguiRenderPass = graphics->GetRenderPass<ImguiRenderPass>();
		m_imguiRenderPass->name = "ImGui";
		m_imguiRenderPass->AddColorAttachment(colorRT, AttachmentUsage::Write, LoadOperation::Load);


		auto computeRP = graphics->GetRenderPass<ComputeRenderPass>();
		auto compute = graphics->CreateComputeShader("Shaders/testCompute.compute.slang", "Test Compute");


		uint32_t grassCount = 10000;


		auto buffer = graphics->CreateGraphicsBuffer(sizeof(glm::vec3) * grassCount, nullptr, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Data.GrassPos");

		computeRP->SetShader(compute);
		computeRP->AddBufferAttachment(buffer, AttachmentUsage::Write);

		//AddRenderPass(computeRP.get());
		AddRenderPass(m_depthRenderPass.get());
		AddRenderPass(m_shadowMapRenderPass.get());
		AddRenderPass(m_forwardRenderPass.get());

		AddRenderPass(m_postProcessingRenderPass.get());


		AddRenderPass(m_imguiRenderPass.get());
		frameGraph.AllocateRequiredResources();
		frameGraph.Compile();

		std::printf(frameGraph.ToString().c_str());
	}
}

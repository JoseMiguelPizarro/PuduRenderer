#include "FrameGraph/FrameGraphRenderPass.h"
#include "RenderFrameData.h"
#include "DrawCall.h"
#include "PuduRenderer.h"
#include "Resources/Resources.h"
#include "SPIRVParser.h"
#include <GPUCommands.h>


namespace Pudu
{
	Pipeline* FrameGraphRenderPass::GetPipeline(RenderFrameData& frameData, DrawCall& drawcall)
	{
		return frameData.renderer->GetOrCreatePipeline(frameData, RenderPassType::Color);
	}

	void FrameGraphRenderPass::Render(RenderFrameData& frameData)
	{
		auto renderScene = frameData.scene;
		auto commands = frameData.currentCommand;

		for (DrawCall drawCall : renderScene->GetDrawCalls()) {

			frameData.currentDrawCall = &drawCall;

			BeforeRenderDrawcall(frameData, drawCall);

			Model model = drawCall.ModelPtr;
			auto mesh = drawCall.MeshPtr;

			Pipeline* pipeline = GetPipeline(frameData, drawCall);
			frameData.graphics->UpdateBindlessResources(pipeline);

			commands->BindPipeline(pipeline);
			if (pipeline->numActiveLayouts > 0)
			{
				vkCmdBindDescriptorSets(commands->vkHandle, pipeline->vkPipelineBindPoint, pipeline->vkPipelineLayoutHandle, 0, pipeline->numActiveLayouts, &pipeline->vkDescriptorSet, 0, nullptr);
			}

			VkBuffer vertexBuffers[] = { mesh->GetVertexBuffer()->Handler };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commands->vkHandle, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commands->vkHandle, mesh->GetIndexBuffer()->Handler, 0, VK_INDEX_TYPE_UINT32);

			auto ubo = frameData.graphics->GetUniformBufferObject(*renderScene->camera, drawCall);


			Viewport viewport;
			viewport.rect = { 0,0, (uint16)frameData.graphics->WindowWidth,(uint16)frameData.graphics->WindowHeight };
			viewport.maxDepth = 1;
			commands->SetViewport(viewport);
			vkCmdPushConstants(commands->vkHandle, pipeline->vkPipelineLayoutHandle, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UniformBufferObject), &ubo);

			if (drawCall.GetRenderMaterial()->Shader->HasFragmentData())
			{
				uint32_t materialid = drawCall.MaterialPtr.Texture->handle.index;
				RenderConstants constants{};
				constants.materialId = materialid;
				constants.g_LightDirection = normalize(vec3(1, 1, 1)); //HARDCODED LIGHT DIRECTION
				vkCmdPushConstants(commands->vkHandle, pipeline->vkPipelineLayoutHandle, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(UniformBufferObject), sizeof(RenderConstants), &constants);
			}

			vkCmdDrawIndexed(commands->vkHandle, static_cast<uint32_t>(mesh->GetIndices()->size()), 1, 0, 0, 0);

			AfterRenderDrawcall(frameData, drawCall);
		}
	}
	PipelineCreationData FrameGraphRenderPass::GetPipelineCreationData(RenderFrameData& frameData, DrawCall& drawcall)
	{
		PipelineCreationData creationData;

		auto shader = frameData.currentDrawCall->MaterialPtr.Shader;

		creationData.vertexShaderData = shader->vertexData;
		creationData.fragmentShaderData = shader->fragmentData;

		BlendStateCreation blendStateCreation;
		blendStateCreation.AddBlendState()
			.SetAlphaBlending(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD)
			.SetColorBlending(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD)
			.SetColorWriteMask(ColorWriteEnabled::All_mask);

		RasterizationCreation rasterizationCreation;
		rasterizationCreation.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationCreation.fill = FillMode::Solid;
		rasterizationCreation.front = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		DepthStencilCreation depthStencilCreation;
		depthStencilCreation.SetDepth(true, VK_COMPARE_OP_LESS_OR_EQUAL);

		VertexInputCreation vertexInputCreation;
		auto attribDescriptions = Vertex::GetAttributeDescriptions();
		auto bindingDescriptions = Vertex::GetBindingDescription();

		for (auto attrib : attribDescriptions)
		{
			VertexAttribute a;
			a.binding = attrib.binding;
			a.format = attrib.format;
			a.location = attrib.location;
			a.offset = attrib.offset;

			vertexInputCreation.AddVertexAttribute(a);
		}

		VertexStream vertexStream;
		vertexStream.binding = bindingDescriptions.binding;
		vertexStream.inputRate = (VertexInputRate::Enum)bindingDescriptions.inputRate;
		vertexStream.stride = bindingDescriptions.stride;

		vertexInputCreation.AddVertexStream(vertexStream);

		ShaderStateCreationData shaderData;
		shaderData.SetName(shader->name.c_str());
		if (shader->HasFragmentData())
		{
			shaderData.AddStage(&shader->fragmentData, shader->fragmentData.size() * sizeof(char), VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		if (shader->HasVertexData())
		{
			shaderData.AddStage(&shader->vertexData, shader->vertexData.size() * sizeof(char), VK_SHADER_STAGE_VERTEX_BIT);
		}

		SPIRVParser::GetDescriptorSetLayout(creationData, creationData.descriptorSetLayoutData);

		creationData.blendState = blendStateCreation;
		creationData.rasterization = rasterizationCreation;
		creationData.depthStencil = depthStencilCreation;
		creationData.vertexInput = vertexInputCreation;
		creationData.shadersStateCreationData = shaderData;

		creationData.renderPassHandle = frameData.currentRenderPass->handle;

		return creationData;
	}
}

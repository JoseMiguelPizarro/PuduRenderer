#include "Shader.h"
#include "FrameGraph/RenderPass.h"
#include "PuduGraphics.h"

namespace Pudu
{
	VkShaderModule Shader::GetModule()
	{
		return VkShaderModule();
	}

	std::filesystem::path Shader::GetPath()
	{
		return std::filesystem::path();
	}

	void Shader::LoadFragmentData(std::vector<char> data)
	{
		m_hasFragmentData = data.size() > 0;

		fragmentData.append_range(data);
	}

	void Shader::LoadVertexData(std::vector<char> data)
	{
		m_hasVertexData = data.size() > 0;

		vertexData.append_range(data);
	}

	SPtr<Pipeline> Shader::CreatePipeline(PuduGraphics* graphics, RenderPass* renderPass)
	{
		PipelineCreationData creationData; //"Question now, how do we populate this?"
		creationData.vertexShaderData = vertexData;
		creationData.fragmentShaderData = fragmentData;
		creationData.name = renderPass->name.c_str();

		BlendStateCreation blendStateCreation;

		if (HasFragmentData())
		{
			blendStateCreation.AddBlendState()
				.SetAlphaBlending(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD)
				.SetColorBlending(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD)
				.SetColorWriteMask(ColorWriteEnabled::All_mask);
		}

		RasterizationCreation rasterizationCreation;
		rasterizationCreation.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationCreation.fill = FillMode::Solid;
		rasterizationCreation.front = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		DepthStencilCreation depthStencilCreation;
		depthStencilCreation.SetDepth(renderPass->writeDepth, VK_COMPARE_OP_LESS_OR_EQUAL);

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
		shaderData.SetName(name.c_str());

		if (HasFragmentData())
		{
			shaderData.AddStage(&fragmentData, fragmentData.size() * sizeof(char),
				VK_SHADER_STAGE_FRAGMENT_BIT);
		}

		if (HasVertexData())
		{
			shaderData.AddStage(&vertexData, vertexData.size() * sizeof(char),
				VK_SHADER_STAGE_VERTEX_BIT);
		}

		creationData.descriptorCreationData = GetDescriptorSetLayouts();
		creationData.blendState = blendStateCreation;
		creationData.rasterization = rasterizationCreation;
		creationData.depthStencil = depthStencilCreation;
		creationData.vertexInput = vertexInputCreation;
		creationData.shadersStateCreationData = shaderData;

		creationData.renderPassHandle = renderPass->Handle();

		auto handle = graphics->CreateGraphicsPipeline(creationData);
		auto pipeline = graphics->Resources()->GetPipeline(handle);

		return pipeline;
	}
}


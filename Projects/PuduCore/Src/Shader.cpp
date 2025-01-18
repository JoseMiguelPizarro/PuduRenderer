#include "Shader.h"
#include "FrameGraph/RenderPass.h"
#include "PuduGraphics.h"

namespace Pudu
{
	void Shader::LoadFragmentData(const uint32_t* data, size_t dataSize, const char* entryPoint)
	{
		m_hasFragmentData = dataSize > 0;

		m_fragmentEntryPoint = entryPoint;
		if (dataSize > 0)
		{
			m_fragmentData =data;
		}
		m_fragmentDataSize = dataSize;
	}

	void Shader::LoadVertexData(const uint32_t* data, size_t dataSize, const char* entryPoint)
	{
		m_hasVertexData = dataSize > 0;
		m_vertexEntryPoint = entryPoint;

		if (dataSize > 0)
		{
			m_vertexData = data;
		}

		m_vertexDataSize = dataSize;
	}

	SPtr<Pipeline> Shader::CreatePipeline(PuduGraphics* graphics, RenderPass* renderPass)
	{
		PipelineCreationData creationData;
		creationData.vertexShaderData = m_vertexData;
		creationData.fragmentShaderData = m_fragmentData;
		creationData.name = renderPass->name.c_str();

		BlendStateCreation blendStateCreation;

		if (HasFragmentData())
		{
			auto renderPassBlendState = renderPass->GetBlendState();
			blendStateCreation.AddBlendState()
				.SetAlphaBlending(renderPassBlendState->sourceAlphaFactor, renderPassBlendState->destinationAlphaFactor, renderPassBlendState->alphaBlendOperation)
				.SetColorBlending(renderPassBlendState->sourceColorFactor, renderPassBlendState->destinationColorFactor, renderPassBlendState->colorBlendOperation)
				.SetColorWriteMask(ColorWriteEnabled::All_mask);
		}

		RasterizationCreation rasterizationCreation;
		rasterizationCreation.cullMode = ToVk(renderPass->GetCullMode());
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
			shaderData.AddStage(m_fragmentData, "fragmentMain", m_fragmentDataSize,
				VK_SHADER_STAGE_FRAGMENT_BIT);
		}

		if (HasVertexData())
		{
			shaderData.AddStage(m_vertexData, "vertexMain", m_vertexDataSize,
				VK_SHADER_STAGE_VERTEX_BIT);
		}

		creationData.blendState = blendStateCreation;
		creationData.rasterization = rasterizationCreation;
		creationData.depthStencil = depthStencilCreation;
		creationData.vertexInput = vertexInputCreation;
		creationData.shadersStateCreationData = shaderData;

		creationData.renderPassHandle = renderPass->Handle();

		creationData.descriptorSetLayouts = &descriptorSetLayouts;
		creationData.activeLayouts = numActiveLayouts;
		creationData.vkDescriptorSetLayout = GetVkDescriptorSetLayouts();

		auto handle = graphics->CreateGraphicsPipeline(creationData);
		auto pipeline = graphics->Resources()->GetPipeline(handle);

		return pipeline;
	}
}


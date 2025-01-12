#pragma once

#include "Resources/GPUResource.h"
#include "Resources/Resources.h"

namespace Pudu
{
	class IShaderObject;
	class RenderPass;
	class Renderer;

	struct PipelineQueryData {
		RenderPass* renderPass;
		IShaderObject* shader;
		Renderer* renderer;
	};

	namespace PipelineType {
		enum Type {
			Graphics,
			Compute
		};
	};

	class Pipeline :public GPUResource<Pipeline>
	{
	public:
		std::string name;
		VkPipeline vkHandle;
		VkPipelineLayout vkPipelineLayoutHandle;

		VkPipelineBindPoint vkPipelineBindPoint;
		VkDescriptorSet vkDescriptorSets[K_MAX_DESCRIPTOR_SET_LAYOUTS];//Just 1 for now, bindless

		uint32_t numDescriptorSets = 0;

		VkFormat depthStencilFormat;

		GPUResourceHandle<ShaderState> shaderState;

		uint32_t numActiveLayouts = 0;

		DepthStencilCreation depthStencil;
		BlendStateCreation blendState;
		RasterizationCreation rasterization;

		bool bindlessUpdated;
		PipelineType::Type pipelineType;

		bool graphicsPipeline = true;
	}; // struct Pipeline
}

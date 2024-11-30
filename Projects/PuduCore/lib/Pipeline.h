#pragma once

#include "Resources/Resources.h"

namespace Pudu
{
	class Shader;
	class RenderPass;
	class PuduRenderer;

	struct PipelineQueryData {
		RenderPass* renderPass;
		Shader* shader;
		PuduRenderer* renderer;
	};

	class Pipeline
	{
	public:
		std::string name;
		VkPipeline vkHandle;
		VkPipelineLayout vkPipelineLayoutHandle;

		VkPipelineBindPoint vkPipelineBindPoint;
		VkDescriptorSet vkDescriptorSets[K_MAX_DESCRIPTOR_SET_LAYOUTS];//Just 1 for now, bindless
		
		uint32_t numDescriptorSets = 0;

		VkFormat depthStencilFormat;

		ShaderStateHandle shaderState;

		const DescriptorSetLayout* descriptorSetLayouts[K_MAX_DESCRIPTOR_SET_LAYOUTS];
		std::vector<DescriptorSetLayoutHandle> descriptorSetLayoutHandles;
		uint32_t numActiveLayouts = 0;

		DepthStencilCreation depthStencil;
		BlendStateCreation blendState;
		RasterizationCreation rasterization;

		bool bindlessUpdated;

		PipelineHandle handle;
		bool graphicsPipeline = true;
	}; // struct Pipeline
}

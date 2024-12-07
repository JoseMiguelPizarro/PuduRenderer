#pragma once

#include "Resources/GPUResource.h"
#include "Resources/Resources.h"

namespace Pudu
{
	class Shader;
	class RenderPass;
	class Renderer;

	struct PipelineQueryData {
		RenderPass* renderPass;
		Shader* shader;
		Renderer* renderer;
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

		SPtr<DescriptorSetLayout> descriptorSetLayouts[K_MAX_DESCRIPTOR_SET_LAYOUTS];
		std::vector<GPUResourceHandle<DescriptorSetLayout>> descriptorSetLayoutHandles;
		uint32_t numActiveLayouts = 0;

		DepthStencilCreation depthStencil;
		BlendStateCreation blendState;
		RasterizationCreation rasterization;

		bool bindlessUpdated;

		bool graphicsPipeline = true;
	}; // struct Pipeline
}

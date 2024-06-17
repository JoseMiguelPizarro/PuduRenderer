#pragma once
#include <filesystem>
#include <memory.h>
#include "PuduCore.h"
#include "Texture2D.h"
#include "Resources/Resources.h"
#include "Resources/ResourcesPool.h"
#include "Shader.h"
#include "Resources/FrameBufferCreationData.h"
#include "ComputeShader.h"

namespace Pudu {
	class PuduGraphics;

	namespace fs = std::filesystem;
	class GPUResourcesManager
	{
	public:
		void Init(PuduGraphics* graphics);
		SPtr<Texture2d> GetTexture(TextureHandle handle);
		SPtr<Texture2d> GetTextureByName(const char* name);
		SPtr<Texture2d> AllocateTexture();

		RenderPass* GetRenderPass(RenderPassHandle handle);
		RenderPassHandle AllocateRenderPass(RenderPassCreationData const& creationData);

		Framebuffer* GetFramebuffer(FramebufferHandle handle);
		FramebufferHandle AllocateFrameBuffer(FramebufferCreationData const& creationData);

		Pipeline* GetPipeline(PipelineHandle handle);
		PipelineHandle AllocatePipeline();

		ShaderStateHandle AllocateShaderState();
		ShaderState* GetShaderState(ShaderStateHandle handle);

		DescriptorSetLayoutHandle AllocateDescriptorSetLayout();
		DescriptorSetLayout* GetDescriptorSetLayout(DescriptorSetLayoutHandle handle);

		ShaderHandle AllocateShader();
		SPtr<Shader> GetShader(ShaderHandle handle);

		ComputeShaderHandle AllocateComputeShader();
		SPtr<ComputeShader> GetComputeShader(ComputeShaderHandle handle);

	private:
		PuduGraphics* m_graphics = nullptr;
		ResourcePool<SPtr<Texture2d>> m_textures;
		ResourcePool<SPtr<Shader>> m_shaders;
		ResourcePool<SPtr<ComputeShader>> m_computeShaders;
		ResourcePool<RenderPass> m_renderPasses;
		ResourcePool<Framebuffer> m_frameBuffers;
		ResourcePool<Pipeline> m_pipelines;
		ResourcePool<ShaderState> m_shaderStates;
		ResourcePool<DescriptorSetLayout> m_descriptorSetLayouts;
		std::unordered_map<std::string, SPtr<Texture2d>> m_texturesByName;
	};
}


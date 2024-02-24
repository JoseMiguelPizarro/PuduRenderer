#include "GPUResourcesManager.h"
#include "Resources/RenderPassCreationData.h"
#include "PuduGraphics.h"
#include "Resources/ResourcesPool.h"

namespace Pudu {

	void GPUResourcesManager::Init(PuduGraphics* graphics)
	{
		m_graphics = graphics;
		m_renderPasses.ObtainResource();
	}

	SPtr<Texture2d> GPUResourcesManager::GetTexture(TextureHandle handle)
	{
		return m_textures.GetResource(handle.index);
	}

	SPtr<Texture2d> GPUResourcesManager::AllocateTexture()
	{
		TextureHandle handle = { static_cast<uint32_t>(m_textures.Size()) };
		SPtr<Texture2d> texture = std::make_shared<Texture2d>();
		texture->handle = handle;
		m_textures.AddResource(texture);

		return texture;
	}

	RenderPass* GPUResourcesManager::GetRenderPass(RenderPassHandle handle)
	{
		return m_renderPasses.GetResourcePtr(handle.index);
	}

	RenderPassHandle GPUResourcesManager::AllocateRenderPass(RenderPassCreationData const& creationData)
	{
		RenderPassHandle handle = { m_renderPasses.ObtainResource() };
		if (handle.index == k_INVALID_HANDLE)
		{
			return handle;
		}

		RenderPass* renderPass = GetRenderPass(handle);

		renderPass->numRenderTargets = creationData.numRenderTargets;
		renderPass->dispatchX = 0;
		renderPass->dispatchY = 0;
		renderPass->dispatchZ = 0;

		renderPass->name = creationData.name;
		renderPass->vkHandle = VK_NULL_HANDLE;

		renderPass->output = RenderPassOutput::GetFromCreationData(m_graphics, creationData);

		m_graphics->CreateRenderPass(renderPass);

		return handle;
	}

	Framebuffer* GPUResourcesManager::GetFramebuffer(FramebufferHandle handle)
	{
		return m_frameBuffers.GetResourcePtr(handle.index);
	}

	FramebufferHandle GPUResourcesManager::AllocateFrameBuffer(FramebufferCreationData const& creationData)
	{
		FramebufferHandle handle = { m_frameBuffers.ObtainResource() };
		if (handle.index == k_INVALID_HANDLE)
		{
			return handle;
		}

		Framebuffer* frameBuffer = GetFramebuffer(handle);
		frameBuffer->numColorAttachments = creationData.numRenderTargets;

		for (uint32_t i = 0; i < creationData.numRenderTargets; i++)
		{
			frameBuffer->colorAttachmentHandles[i] = creationData.outputTexturesHandle[i];
		}

		frameBuffer->depthStencilAttachmentHandle = creationData.depthStencilTextureHandle;
		frameBuffer->width = creationData.width;
		frameBuffer->height = creationData.height;
		frameBuffer->scaleX = creationData.scaleX;
		frameBuffer->scaleY = creationData.scaleY;
		frameBuffer->resize = creationData.resize;
		frameBuffer->name = creationData.name;
		frameBuffer->renderPassHandle = creationData.renderPassHandle;

		m_graphics->CreateVkFramebuffer(frameBuffer);

		return handle;
	}

	Pipeline* GPUResourcesManager::GetPipeline(PipelineHandle handle)
	{
		return m_pipelines.GetResourcePtr(handle.index);
	}

	PipelineHandle GPUResourcesManager::AllocatePipeline()
	{
		return { m_pipelines.ObtainResource() };
	}

	ShaderStateHandle GPUResourcesManager::AllocateShaderState()
	{
		return { m_shaderStates.ObtainResource() };
	}

	ShaderState* GPUResourcesManager::GetShaderState(ShaderStateHandle handle)
	{
		return m_shaderStates.GetResourcePtr(handle.index);
	}

	DescriptorSetLayoutHandle GPUResourcesManager::AllocateDescriptorSetLayout()
	{
		return { m_descriptorSetLayouts.ObtainResource() };
	}

	DescriptorSetLayout* GPUResourcesManager::GetDescriptorSetLayout(DescriptorSetLayoutHandle handle)
	{
		return m_descriptorSetLayouts.GetResourcePtr(handle.index);
	}

	ShaderHandle GPUResourcesManager::AllocateShader()
	{
		return { m_shaders.ObtainResource() };
	}

	Shader* GPUResourcesManager::GetShader(ShaderHandle handle)
	{
		return m_shaders.GetResourcePtr(handle.index);
	}
}

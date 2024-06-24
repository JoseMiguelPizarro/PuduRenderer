#include "GPUResourcesManager.h"
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

	SPtr<Texture2d> GPUResourcesManager::GetTextureByName(const char* name)
	{
		if (name == nullptr)
		{
			return nullptr;
		}

		for (auto texture : m_textures.m_resources) {
			if (texture->name.empty())
			{
				continue;
			}
			if (texture->name.compare(name) == 0)
			{
				return texture;
			}
		}

		return nullptr;
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

	RenderPass* GPUResourcesManager::AllocateRenderPass(RenderPassCreationData const & creationData)
	{
		RenderPass renderPass;

		if (creationData.isCompute)
		{
			renderPass = ComputeRenderPass();
		}
		else
		{
			renderPass = RenderPass();
		}

		RenderPassHandle handle = { m_renderPasses.AddResource(renderPass)};

		if (handle.index == k_INVALID_HANDLE)
		{
			return nullptr;
		}

		RenderPass* renderPassPtr = GetRenderPass(handle);
		renderPassPtr->handle = handle;

		return renderPassPtr;
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
		ShaderHandle handle = { static_cast<uint32_t>(m_shaders.Size()) };
		SPtr<Shader> shader = std::make_shared<Shader>();

		shader->handle = handle;
		m_shaders.AddResource(shader);


		return { handle };
	}

	SPtr<Shader> GPUResourcesManager::GetShader(ShaderHandle handle)
	{
		return m_shaders.GetResource(handle.index);
	}

	ComputeShaderHandle GPUResourcesManager::AllocateComputeShader()
	{
		ComputeShaderHandle handle = { static_cast<uint32_t>(m_computeShaders.Size()) };
		SPtr<ComputeShader> shader = std::make_shared<ComputeShader>();

		shader->handle = handle;

		m_computeShaders.AddResource(shader);

		return { handle };
	}

	SPtr<ComputeShader> GPUResourcesManager::GetComputeShader(ComputeShaderHandle handle)
	{
		return m_computeShaders.GetResource(handle.index);
	}
}

#include "GPUResourcesManager.h"
#include "PuduGraphics.h"
#include "Resources/ResourcesPool.h"

namespace Pudu {

	void GPUResourcesManager::Init(PuduGraphics* graphics)
	{
		m_graphics = graphics;
	}

	SPtr<Texture> GPUResourcesManager::GetTextureByName(const char* name)
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

	SPtr<Texture2d> GPUResourcesManager::AllocateTexture2D()
	{
		return AllocateGPUResource<Texture2d>(m_textures);
	}

	SPtr<TextureCube> GPUResourcesManager::AllocateTextureCube()
	{
		return AllocateGPUResource<TextureCube>(m_textures);
	}

	SPtr<RenderPass> GPUResourcesManager::GetRenderPass(GPUResourceHandle<RenderPass> handle)
	{
		return m_renderPasses.GetResource(handle);
	}

	SPtr<Framebuffer> GPUResourcesManager::GetFramebuffer(GPUResourceHandle<Framebuffer> handle)
	{
		return m_frameBuffers.GetResource(handle);
	}

	SPtr<Framebuffer> GPUResourcesManager::AllocateFrameBuffer()
	{
		return AllocateGPUResource<Framebuffer>(m_frameBuffers);
	}

	SPtr<Pipeline> GPUResourcesManager::GetPipeline(GPUResourceHandle<Pipeline> handle)
	{
		return m_pipelines.GetResource(handle);
	}

	SPtr<Pipeline>  GPUResourcesManager::AllocatePipeline()
	{
		return AllocateGPUResource<Pipeline>(m_pipelines);
	}

	SPtr<ShaderState> GPUResourcesManager::AllocateShaderState()
	{
		return AllocateGPUResource<ShaderState>(m_shaderStates);
	}

	SPtr<ShaderState> GPUResourcesManager::GetShaderState(GPUResourceHandle<ShaderState> handle)
	{
		return m_shaderStates.GetResource(handle);
	}

	SPtr<DescriptorSetLayout> GPUResourcesManager::AllocateDescriptorSetLayout()
	{
		return AllocateGPUResource<DescriptorSetLayout>(m_descriptorSetLayouts);
	}

	SPtr<DescriptorSetLayout>GPUResourcesManager::GetDescriptorSetLayout(GPUResourceHandle<DescriptorSetLayout> handle)
	{
		return m_descriptorSetLayouts.GetResource(handle);
	}

	SPtr<Shader> GPUResourcesManager::AllocateShader()
	{
		return AllocateGPUResource<Shader>(m_shaders);
	}

	SPtr<Shader> GPUResourcesManager::GetShader(GPUResourceHandle<Shader> handle)
	{
		return m_shaders.GetResource(handle);
	}

	SPtr<Mesh> GPUResourcesManager::AllocateMesh()
	{
		return AllocateGPUResource<Mesh>(m_meshes);
	}

	SPtr<Mesh> GPUResourcesManager::GetMesh(GPUResourceHandle<Mesh> handle)
	{
		return m_meshes.GetResource(handle);
	}

	SPtr<ComputeShader> GPUResourcesManager::AllocateComputeShader()
	{
		return AllocateGPUResource<ComputeShader>(m_computeShaders);
	}

	SPtr<ComputeShader> GPUResourcesManager::GetComputeShader(GPUResourceHandle<ComputeShader> handle)
	{
		return m_computeShaders.GetResource(handle);
	}
	SPtr<GraphicsBuffer> GPUResourcesManager::AllocateGraphicsBuffer()
	{
		return AllocateGPUResource<GraphicsBuffer>(m_graphicsBuffers);
	}
	SPtr<GraphicsBuffer> GPUResourcesManager::GetGraphicsBuffer(GPUResourceHandle<GraphicsBuffer> handle)
	{
		return m_graphicsBuffers.GetResource(handle);
	}
	SPtr<Semaphore> GPUResourcesManager::AllocateSemaphore()
	{
		return AllocateGPUResource<Semaphore>(m_semaphores);
	}
	SPtr<Semaphore> GPUResourcesManager::GetSemaphore(GPUResourceHandle<Semaphore> handle)
	{
		return m_semaphores.GetResource(handle);
	}

	SPtr<GPUCommands> GPUResourcesManager::AllocateCommandBuffer()
	{
		return AllocateGPUResource<GPUCommands>(m_commandBuffers);
	}

	SPtr<GPUCommands> GPUResourcesManager::GetComandBuffer(GPUResourceHandle<GPUCommands> handle)
	{
		return m_commandBuffers.GetResource(handle);
	}

	SPtr<RenderTexture> GPUResourcesManager::AllocateRenderTexture()
	{
		return AllocateGPUResource<RenderTexture>(m_textures);
	}

	SPtr<RenderTexture> GPUResourcesManager::GetRenderTexture(GPUResourceHandle<RenderTexture> handle)
	{
		return std::static_pointer_cast<RenderTexture>(m_textures.GetResource(handle));
	}

	SPtr<CommandPool> GPUResourcesManager::AllocateCommandPool()
	{
		return AllocateGPUResource<CommandPool>(m_commandPools);
	}

	SPtr<CommandPool> GPUResourcesManager::GetCommandPool(GPUResourceHandle<CommandPool> handle)
	{
		return m_commandPools.GetResource(handle);
	}

	void GPUResourcesManager::DestroyAllResources(PuduGraphics* gfx)
	{
		for (auto& t : m_textures.m_resources)
		{
			gfx->DestroyTexture(t);
		}

		for (auto& t : m_pipelines.m_resources) {
			vkDestroyPipeline(gfx->m_device, t->vkHandle, nullptr);
			vkDestroyPipelineLayout(gfx->m_device, t->vkPipelineLayoutHandle, gfx->m_allocatorPtr);
		}

		for (auto& f : m_frameBuffers.m_resources)
		{
			gfx->DestroyFrameBuffer(f);
		}

		for (auto& rp : m_renderPasses.m_resources)
		{
			gfx->DestroyRenderPass(rp);
		}

		for (auto& m : m_meshes.m_resources)
		{
			gfx->DestroyMesh(m);
		}

		for (auto& b : m_graphicsBuffers.m_resources)
		{
			gfx->DestroyBuffer(b);
		}

		for (auto& s : m_semaphores.m_resources)
		{
			gfx->DestroySemaphore(s);
		}

		for (auto& s : m_shaders.m_resources) {
			gfx->DestroyShader(s);
		}

		for (auto& ss : m_shaderStates.m_resources) {
			for (size_t i = 0; i < ss->activeShaders; i++)
			{
				auto a = ss->shaderStageInfo[i];

				gfx->DestroyShaderModule(a.module);
			}
		}

		for (auto& ds : m_descriptorSetLayouts.m_resources) {

			gfx->DestroyDescriptorSetLayout(*ds);
		}

		for (auto& cp : m_commandPools.m_resources) {
			gfx->DestroyCommandPool(cp.get());
		}

	}
}

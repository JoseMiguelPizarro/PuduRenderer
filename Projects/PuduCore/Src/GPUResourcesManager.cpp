#include "GPUResourcesManager.h"
#include "PuduGraphics.h"
#include "Resources/ResourcesPool.h"
#include "FrameGraph/ForwardRenderPass.h"
#include "FrameGraph/ShadowMapRenderPass.h"
#include "FrameGraph/DepthStencilRenderPass.h"
#include "Logger.h"

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
		TextureHandle handle = { static_cast<uint32_t>(m_textures.Size()) };
		SPtr<Texture2d> texture = std::make_shared<Texture2d>();
		texture->handle = handle;

		m_textures.AddResource(texture);

		return texture;
	}

	SPtr<TextureCube> GPUResourcesManager::AllocateTextureCube()
	{
		TextureHandle handle = { static_cast<uint32_t>(m_textures.Size()) };
		SPtr<TextureCube> texture = std::make_shared<TextureCube>();
		texture->handle = handle;

		m_textures.AddResource(texture);

		return texture;
	}

	SPtr<RenderPass> GPUResourcesManager::GetRenderPass(RenderPassHandle handle)
	{
		return m_renderPasses.GetResource(handle.index);
	}

	SPtr<RenderPass> GPUResourcesManager::AllocateRenderPass(RenderPassType const& renderPassType)
	{
		RenderPassHandle handle;

		switch (renderPassType)
		{
		case RenderPassType::Color:
			handle = { m_renderPasses.AddResource(std::make_shared<RenderPass>()) };
			break;
		case RenderPassType::ShadowMap:
			handle = { m_renderPasses.AddResource(std::make_shared<ShadowMapRenderPass>()) };
			break;
		case RenderPassType::DepthPrePass:
			handle = { m_renderPasses.AddResource(std::make_shared <DepthStencilRenderPass>()) };
			break;
		default:
			handle = { m_renderPasses.AddResource(std::make_shared<RenderPass>()) };
			break;
		}

		auto renderPass = m_renderPasses.GetResource(handle.index);

		renderPass->handle = handle;

		return renderPass;
	}

	SPtr<Framebuffer> GPUResourcesManager::GetFramebuffer(FramebufferHandle handle)
	{
		return m_frameBuffers.GetResource(handle.index);
	}

	SPtr<Framebuffer> GPUResourcesManager::AllocateFrameBuffer()
	{
		FramebufferHandle handle =  { static_cast<uint32_t>(m_frameBuffers.Size()) };
		SPtr<Framebuffer> framebuffer = std::make_shared<Framebuffer>();

		m_frameBuffers.AddResource(framebuffer);

		framebuffer->handle = handle;

		return framebuffer;
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

	SPtr<Mesh> GPUResourcesManager::AllocateMesh()
	{
		MeshHandle handle = { static_cast<uint32_t>(m_meshes.Size()) };
		SPtr<Mesh> mesh = std::make_shared<Mesh>();

		mesh->handle = handle;

		m_meshes.AddResource(mesh);

		return mesh;
	}

	SPtr<Mesh> GPUResourcesManager::GetMesh(MeshHandle handle)
	{
		return m_meshes.GetResource(handle.index);
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
	SPtr<GraphicsBuffer> GPUResourcesManager::AllocateGraphicsBuffer()
	{
		GraphicsBufferHandle handle = { static_cast<uint32_t>(m_graphicsBuffers.Size()) };
		SPtr<GraphicsBuffer> buffer = std::make_shared<GraphicsBuffer>();

		buffer->handle = handle;

		m_graphicsBuffers.AddResource(buffer);

		return buffer;
	}
	SPtr<GraphicsBuffer> GPUResourcesManager::GetGraphicsBuffer(GraphicsBufferHandle handle)
	{
		return m_graphicsBuffers.GetResource(handle.index);
	}
	SPtr<Semaphore> GPUResourcesManager::AllocateSemaphore()
	{
		SemaphoreHandle semaphore = { static_cast<uint32_t>(m_semaphores.Size()) };
		SPtr<Semaphore> semaphoreptr = std::make_shared<Semaphore>();

		semaphoreptr->handle = semaphore;

		m_semaphores.AddResource(semaphoreptr);

		return semaphoreptr;
	}
	SPtr<Semaphore> GPUResourcesManager::GetSemaphore(SemaphoreHandle handle)
	{
		return m_semaphores.GetResource(handle.index);
	}

	SPtr<GPUCommands> GPUResourcesManager::AllocateCommandBuffer()
	{
		return AllocateGPUResource<GPUCommands>(m_commandBuffers);
	}

	SPtr<GPUCommands> GPUResourcesManager::GetComandBuffer(GPUResourceHandle handle)
	{
		return m_commandBuffers.GetResource(handle.index);
	}

	void GPUResourcesManager::DestroyAllResources(PuduGraphics* gfx)
	{
		for (auto t : m_textures.m_resources)
		{
			gfx->DestroyTexture(t);
		}

		for (auto t : m_pipelines.m_resources) {
			vkDestroyPipeline(gfx->m_device, t.vkHandle, nullptr);
		}

		for (auto f : m_frameBuffers.m_resources)
		{
			gfx->DestroyFrameBuffer(f);
		}

		for (auto rp : m_renderPasses.m_resources)
		{
			gfx->DestroyRenderPass(rp);
		}

		for (auto m : m_meshes.m_resources)
		{
			gfx->DestroyMesh(m);
		}

		for (auto b : m_graphicsBuffers.m_resources)
		{
			gfx->DestroyBuffer(b);
		}

		for (auto s : m_semaphores.m_resources)
		{
			gfx->DestroySemaphore(s);
		}

		for (auto s : m_shaders.m_resources) {
			gfx->DestroyShader(s);
		}

		for (auto ss : m_shaderStates.m_resources) {
			for (size_t i = 0; i < ss.activeShaders; i++)
			{
				auto a = ss.shaderStageInfo[i];
				
				gfx->DestroyShaderModule(a.module);
			}
		}

		for (auto& ds : m_descriptorSetLayouts.m_resources) {

			gfx->DestroyDescriptorSetLayout(ds);
		}

	}
}

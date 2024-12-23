#pragma once
#pragma once
#include <filesystem>
#include <memory.h>
#include <concepts>

#include "PuduCore.h"
#include "Texture.h"
#include "Texture2D.h"
#include "TextureCube.h"
#include "Resources/Resources.h"
#include "Resources/ResourcesPool.h"
#include "FrameGraph/RenderPass.h"
#include "Shader.h"
#include "Resources/FrameBufferCreationData.h"
#include "ComputeShader.h"
#include "Pipeline.h"
#include "Logger.h"
#include <Semaphore.h>
#include <Resources/GPUResource.h>
#include "Resources/CommandPool.h"
#include "Resources/DescriptorPool.h"

namespace Pudu {
	class PuduGraphics;

	namespace fs = std::filesystem;
	class GPUResourcesManager
	{
	public:
		void Init(PuduGraphics* graphics);


		SPtr<Texture> GetTextureByName(const char* name);
		SPtr<Texture2d> AllocateTexture2D();
		SPtr<TextureCube> AllocateTextureCube();

		SPtr<RenderPass> GetRenderPass(GPUResourceHandle<RenderPass> handle);

		SPtr<Framebuffer> GetFramebuffer(GPUResourceHandle<Framebuffer> handle);
		SPtr<Framebuffer> AllocateFrameBuffer();

		SPtr<Pipeline> AllocatePipeline();
		SPtr<Pipeline> GetPipeline(GPUResourceHandle<Pipeline> handle);

		SPtr<ShaderState> AllocateShaderState();
		SPtr<ShaderState> GetShaderState(GPUResourceHandle<ShaderState> handle);

		SPtr<DescriptorSetLayout> AllocateDescriptorSetLayout();
		SPtr<DescriptorSetLayout> GetDescriptorSetLayout(GPUResourceHandle<DescriptorSetLayout> handle);

		SPtr<Shader> AllocateShader();
		SPtr<Shader> GetShader(GPUResourceHandle<Shader> handle);

		SPtr<Mesh> AllocateMesh();
		SPtr<Mesh> GetMesh(GPUResourceHandle<Mesh> handle);

		SPtr<ComputeShader> AllocateComputeShader();
		SPtr<ComputeShader> GetComputeShader(GPUResourceHandle<ComputeShader> handle);

		SPtr<GraphicsBuffer> AllocateGraphicsBuffer();
		SPtr<GraphicsBuffer> GetGraphicsBuffer(GPUResourceHandle<GraphicsBuffer> handle);

		SPtr<Semaphore> AllocateSemaphore();
		SPtr<Semaphore> GetSemaphore(GPUResourceHandle<Semaphore> handle);

		SPtr<GPUCommands> AllocateCommandBuffer();
		SPtr<GPUCommands> GetComandBuffer(GPUResourceHandle<GPUCommands> handle);

		SPtr<RenderTexture> AllocateRenderTexture();
		SPtr<RenderTexture> GetRenderTexture(GPUResourceHandle<RenderTexture> handle);

		SPtr<CommandPool> AllocateCommandPool();
		SPtr<CommandPool> GetCommandPool(GPUResourceHandle<CommandPool> handle);

		SPtr<DescriptorPool> AllocateDescriptorPool();
		SPtr<DescriptorPool> GetDescriptorPool(GPUResourceHandle<DescriptorPool> handle);

		void DestroyAllResources(PuduGraphics* gfx);


		template<class T>
			requires (std::convertible_to<T, Texture>)
		SPtr<T> GetTexture(GPUResourceHandle<Texture> handle)
		{
			return static_pointer_cast<T>(m_textures.GetResource(handle.Index()));
		}

		template<typename T, typename poolType>
		//	requires(std::convertible_to < T, GPUResource<T>>)
		SPtr<T> AllocateGPUResource(ResourcePool<SPtr<poolType>>& pool) {

			uint32_t resourceIndex = { static_cast<uint32_t>(pool.Size()) };
			SPtr<T> resourcePtr = std::make_shared<T>();

			resourcePtr->m_handle.m_Index = resourceIndex;

			pool.AddResource(resourcePtr);

			return resourcePtr;
		}

		template<class T>
			requires (std::convertible_to<T, RenderPass>)
		SPtr<T> AllocateRenderPass() {
			return AllocateGPUResource<T>(m_renderPasses);
		}


		ResourcePool<SPtr<Texture>>* GetAllocatedTextures() { return &m_textures; }

	private:
		friend class PuduGraphics;
		friend class Renderer;

		PuduGraphics* m_graphics = nullptr;
		ResourcePool<SPtr<Texture>> m_textures;
		ResourcePool<SPtr<Shader>> m_shaders;
		ResourcePool<SPtr<ShaderState>> m_shaderStates;
		ResourcePool<SPtr<Mesh>> m_meshes;
		ResourcePool<SPtr<ComputeShader>> m_computeShaders;
		ResourcePool<SPtr<RenderPass>> m_renderPasses;
		ResourcePool<SPtr<Framebuffer>> m_frameBuffers;
		ResourcePool<SPtr<Pipeline>> m_pipelines;
		ResourcePool<SPtr<GraphicsBuffer>> m_graphicsBuffers;
		ResourcePool<SPtr<DescriptorSetLayout>> m_descriptorSetLayouts;
		ResourcePool<SPtr<Semaphore>> m_semaphores;
		ResourcePool<SPtr<GPUCommands>> m_commandBuffers;
		ResourcePool<SPtr<CommandPool>> m_commandPools;
		ResourcePool<SPtr<DescriptorPool>> m_descriptorPools;

		std::unordered_map<std::string, SPtr<Texture>> m_texturesByName;
	};
}


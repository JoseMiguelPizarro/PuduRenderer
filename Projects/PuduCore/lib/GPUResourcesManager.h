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

		SPtr<RenderPass> GetRenderPass(GPUResourceHandle handle);

		SPtr<Framebuffer> GetFramebuffer(FramebufferHandle handle);
		SPtr<Framebuffer> AllocateFrameBuffer();

		Pipeline* GetPipeline(PipelineHandle handle);
		PipelineHandle AllocatePipeline();

		ShaderStateHandle AllocateShaderState();
		ShaderState* GetShaderState(ShaderStateHandle handle);

		DescriptorSetLayoutHandle AllocateDescriptorSetLayout();
		DescriptorSetLayout* GetDescriptorSetLayout(DescriptorSetLayoutHandle handle);

		ShaderHandle AllocateShader();
		SPtr<Shader> GetShader(ShaderHandle handle);

		SPtr<Mesh> AllocateMesh();
		SPtr<Mesh> GetMesh(MeshHandle handle);

		ComputeShaderHandle AllocateComputeShader();
		SPtr<ComputeShader> GetComputeShader(ComputeShaderHandle handle);

		SPtr<GraphicsBuffer> AllocateGraphicsBuffer();
		SPtr<GraphicsBuffer> GetGraphicsBuffer(GraphicsBufferHandle handle);

		SPtr<Semaphore> AllocateSemaphore();
		SPtr<Semaphore> GetSemaphore(SemaphoreHandle handle);

		SPtr<GPUCommands> AllocateCommandBuffer();
		SPtr<GPUCommands> GetComandBuffer(GPUResourceHandle handle);

		SPtr<RenderTexture> AllocateRenderTexture();
		SPtr<RenderTexture> GetRenderTexture(GPUResourceHandle handle);

		void DestroyAllResources(PuduGraphics* gfx);


		template<class T>
			requires (std::convertible_to<T, Texture>)
		SPtr<T> GetTexture(GPUResourceHandle handle)
		{
			LOG("Getting resource {}", handle.index);
			return static_pointer_cast<T>(m_textures.GetResource(handle.index));

			PUDU_ERROR("Trying to get a texture from a type not yet supported {}", typeid(T).name());
		}



		template<typename T, typename poolType>
			requires(std::convertible_to<T, GPUResource>)
		SPtr<T> AllocateGPUResource(ResourcePool<SPtr<poolType>>& pool) {

			uint32_t resourceIndex = { static_cast<uint32_t>(pool.Size()) };
			SPtr<T> resourcePtr = std::make_shared<T>();

			resourcePtr->m_handle.index = resourceIndex;

			pool.AddResource(resourcePtr);

			return resourcePtr;
		}

		template<class T>
			requires (std::convertible_to<T, RenderPass>)
		SPtr<T> AllocateRenderPass() {
			return AllocateGPUResource<T>(m_renderPasses);
		}

	private:
		PuduGraphics* m_graphics = nullptr;
		ResourcePool<SPtr<Texture>> m_textures;
		ResourcePool<SPtr<Shader>> m_shaders;
		ResourcePool<SPtr<Mesh>> m_meshes;
		ResourcePool<SPtr<ComputeShader>> m_computeShaders;
		ResourcePool<SPtr<RenderPass>> m_renderPasses;
		ResourcePool<SPtr<Framebuffer>> m_frameBuffers;
		ResourcePool<Pipeline> m_pipelines;
		ResourcePool<ShaderState> m_shaderStates;
		ResourcePool<SPtr<GraphicsBuffer>> m_graphicsBuffers;
		ResourcePool<DescriptorSetLayout> m_descriptorSetLayouts;
		ResourcePool<SPtr<Semaphore>> m_semaphores;
		ResourcePool<SPtr<GPUCommands>> m_commandBuffers;
		std::unordered_map<std::string, SPtr<Texture>> m_texturesByName;
	};
}


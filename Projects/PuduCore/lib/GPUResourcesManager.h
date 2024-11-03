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

		SPtr<RenderPass> GetRenderPass(RenderPassHandle handle);
		SPtr<RenderPass> AllocateRenderPass(RenderPassCreationData const& creationdata);

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

		SPtr<Mesh> AllocateMesh();
		SPtr<Mesh> GetMesh(MeshHandle handle);

		ComputeShaderHandle AllocateComputeShader();
		SPtr<ComputeShader> GetComputeShader(ComputeShaderHandle handle);

		SPtr<GraphicsBuffer> AllocateGraphicsBuffer();
		SPtr<GraphicsBuffer> GetGraphicsBuffer(GraphicsBufferHandle handle);

		SPtr<Semaphore> AllocateSemaphore();
		SPtr<Semaphore> GetSemaphore(SemaphoreHandle handle);

		void DestroyAllResources(PuduGraphics* gfx);


		template<class T>
			requires (std::convertible_to<T, Texture>)
		SPtr<T> GetTexture(TextureHandle handle)
		{
			return static_pointer_cast<T>(m_textures.GetResource(handle.index));

			PUDU_ERROR("Trying to get a texture from a type not yet supported {}", typeid(T).name());
		}

	private:
		PuduGraphics* m_graphics = nullptr;
		ResourcePool<SPtr<Texture>> m_textures;
		ResourcePool<SPtr<Shader>> m_shaders;
		ResourcePool<SPtr<Mesh>> m_meshes;
		ResourcePool<SPtr<ComputeShader>> m_computeShaders;
		ResourcePool<SPtr<RenderPass>> m_renderPasses;
		ResourcePool<Framebuffer> m_frameBuffers;
		ResourcePool<Pipeline> m_pipelines;
		ResourcePool<ShaderState> m_shaderStates;
		ResourcePool<SPtr<GraphicsBuffer>> m_graphicsBuffers;
		ResourcePool<DescriptorSetLayout> m_descriptorSetLayouts;
		ResourcePool<SPtr<Semaphore>> m_semaphores;
		std::unordered_map<std::string, SPtr<Texture>> m_texturesByName;
	};
}


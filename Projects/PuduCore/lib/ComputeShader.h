#pragma once
#include "Resources/Resources.h"
#include "ShaderObject.h"
#include "Material.h"
#include <GraphicsBuffer.h>

namespace Pudu
{
	class Pipeline;

	struct ComputeShaderCreationData
	{
		fs::path shaderPath;
		std::string name;
		std::string kernel = "computeMain";
	};

	class ComputeShader :public GPUResource<ComputeShader>, public IShaderObject
	{
	public:
		GPUResourceHandle<ComputeShader> handle;
		bool ResourcesUpdated() { return m_resourcesUpdated; }
		void MarkAsResourcesUpdated() { m_resourcesUpdated = true; }
		void MarkResourcesDirty() { m_resourcesUpdated = false; }
		void SetTexture(const char* name, SPtr<Texture> texture);
		void SetBuffer(const char* name, SPtr<GraphicsBuffer> buffer);
		ShaderPropertiesBlock* GetPropertiesBlock() { return &m_propertiesBlock; }
		GPUResourceHandle<Pipeline> GetPipelineHandle() { return m_pipelineHandle; };
		void SetKernel(const char* name) { m_kernel = name; };
		const char* GetKernel() { return m_kernel.c_str(); };
		void SetName(const char* name) override { this->name = name; };
		const char* GetName() override { return this->name.c_str(); };

	protected:
		SPtr<Pipeline> OnCreatePipeline(PuduGraphics* graphics, RenderPass* renderPass) override;
	private:
		friend class PuduGraphics;
		bool m_resourcesUpdated = false;
		GPUResourceHandle<Pipeline> m_pipelineHandle;
		ShaderPropertiesBlock m_propertiesBlock;
		std::string m_kernel;
	};
}



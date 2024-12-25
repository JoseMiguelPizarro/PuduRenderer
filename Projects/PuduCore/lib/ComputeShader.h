#pragma once
#include "Resources/Resources.h"
#include "ShaderObject.h"
#include "Material.h"
#include <GraphicsBuffer.h>

namespace Pudu
{
	class Pipeline;

	class ComputeShader :public GPUResource<ComputeShader>, public IShaderObject
	{
	public:
		GPUResourceHandle<ComputeShader> handle;
		bool ResourcesUpdated() { return m_resourcesUpdated; }
		void MarkAsResourcesUpdated() { m_resourcesUpdated = true; }
		void MarkResourcesDirty() { m_resourcesUpdated = false; }
		void SetTexture(const char* name, SPtr<Texture> texture);
		void SetBuffer(const char* name, SPtr<GraphicsBuffer> buffer);
		SPtr<Pipeline> CreatePipeline(PuduGraphics* graphics, RenderPass* renderPass) override;
		ShaderPropertiesBlock* GetPropertiesBlock() { return &m_propertiesBlock; }
		GPUResourceHandle<Pipeline> GetPipelineHandle() { return m_pipelineHandle; };


	private:
		friend class PuduGraphics;
		bool m_resourcesUpdated = false;
		GPUResourceHandle<Pipeline> m_pipelineHandle;
		ShaderPropertiesBlock m_propertiesBlock;
	};
}



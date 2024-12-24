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
		GPUResourceHandle<Pipeline> pipelineHandle;
		GPUResourceHandle<ComputeShader> handle;
		bool ResourcesUpdated() { return m_resourcesUpdated; }
		void MarkAsResourcesUpdated() { m_resourcesUpdated = true; }
		void MarkResourcesDirty() { m_resourcesUpdated = false; }
		void SetTexture(const char* name, SPtr<Texture> texture);
		void SetBuffer(const char* name, SPtr<GraphicsBuffer> buffer);
		ShaderPropertiesBlock* GetPropertiesBlock() { return &m_propertiesBlock; }

	private:
		friend class PuduGraphics;
		bool m_resourcesUpdated = false;
		ShaderPropertiesBlock m_propertiesBlock;
	};
}



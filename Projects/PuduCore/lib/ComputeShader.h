#pragma once

#include "Resources/Resources.h"


namespace Pudu
{
	class Pipeline;

	class ComputeShader :public GPUResource<ComputeShader>
	{
	public:
		VkShaderModule vkShaderModule;
		GPUResourceHandle<Pipeline> pipelineHandle;
		GPUResourceHandle<ComputeShader> handle;
		bool ResourcesUpdated() { return m_resourcesUpdated; }
		void MarkAsResourcesUpdated() { m_resourcesUpdated = true; }
		void MarkResourcesDirty() { m_resourcesUpdated = false; }

	private:
		bool m_resourcesUpdated = false;
	};
}



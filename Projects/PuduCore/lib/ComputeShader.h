#pragma once

#include "Resources/Resources.h"


namespace Pudu
{
	class ComputeShader
	{
	public:
		VkShaderModule vkShaderModule;
		PipelineHandle pipelineHandle;
		ComputeShaderHandle handle;
		bool ResourcesUpdated() { return m_resourcesUpdated; }
		void MarkAsResourcesUpdated() { m_resourcesUpdated = true; }
		void MarkResourcesDirty() { m_resourcesUpdated = false; }

	private:
		bool m_resourcesUpdated = false;
	};
}



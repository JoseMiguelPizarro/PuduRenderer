#pragma once
#include <string>
#include <filesystem>
#include <vulkan/vulkan_core.h>
#include "Resources/GPUResource.h"
#include  "Resources/Resources.h"
#include "ShaderObject.h"

namespace Pudu
{
	class Pipeline;
	class RenderPass;
	class PuduGraphics;

	class Shader : public GPUResource<Shader>, public IShaderObject
	{
	public:
		~Shader() override
		{
			delete m_fragmentData;
			delete m_vertexData;
		}

		Shader() = default;

		void LoadFragmentData(const uint32_t* data, size_t dataSize, const char* entryPoint = "main");
		void LoadVertexData(const uint32_t* data, size_t dataSize, const char* entryPoint = "main");

		const uint32_t* GetFragmentData() const { return m_fragmentData; }
		const uint32_t* GetVertexData() const { return m_vertexData; }
		size_t GetVertexDataSize() const { return m_vertexDataSize; }
		size_t GetFragmentDataSize() const { return m_fragmentDataSize; }

		bool HasFragmentData() const { return m_hasFragmentData; }
		bool HasVertexData() const { return m_hasVertexData; }

		const char* GetFragmentEntryPoint() const { return m_fragmentEntryPoint.c_str(); }
		const char* GetVertexEntryPoint() const { return m_vertexEntryPoint.c_str(); }

		void SetName(const char* name) override { this->name = name; };
		const char* GetName() override { return this->name.c_str(); };

		SPtr<Pipeline> CreatePipeline(PuduGraphics* gfx, RenderPass* renderPass) override;

	private:
		friend class PuduGraphics;
		std::filesystem::path m_shaderPath;
		bool m_hasFragmentData;
		bool m_hasVertexData;
		std::string m_fragmentEntryPoint;
		std::string m_vertexEntryPoint;

		const uint32_t* m_fragmentData;
		size_t m_fragmentDataSize;
		const uint32_t* m_vertexData;
		size_t m_vertexDataSize;
	};
}

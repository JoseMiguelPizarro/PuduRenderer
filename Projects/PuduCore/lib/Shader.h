#pragma once
#include <string>
#include <filesystem>
#include "Resources/GPUResource.h"
#include  "Resources/Resources.h"
#include "ShaderObject.h"

namespace Pudu
{
    class Pipeline;
    class RenderPass;
    class PuduGraphics;

    struct ShaderData
    {
        const u32* data;
        Size size;
        std::string entryPoint;
    };

    class Shader : public GPUResource<Shader>, public IShaderObject
    {
    public:
        ~Shader() override
        {
            if (m_fragmentData.has_value())
                delete m_fragmentData.value().data;
            if (m_vertexData.has_value())
                delete m_vertexData.value().data;
            if (m_geometryData.has_value())
                delete m_geometryData.value().data;
        }

        Shader() = default;

        void LoadFragmentData(const u32* data, Size dataSize, const char* entryPoint = K_FRAGMENT_SHADER_ENTRY_POINT);
        void LoadVertexData(const u32* data, Size dataSize, const char* entryPoint = K_VERTEX_SHADER_ENTRY_POINT);
        void LoadGeometryData(const u32* data, Size dataSize, const char* entryPoint = K_GEOMETRY_SHADER_ENTRY_POINT);

        const u32* GetVertexData() const { return m_vertexData.value().data; }
        const u32* GetGeometryData() const { return m_geometryData.value().data; }
        const u32* GetFragmentData() const { return m_fragmentData.value().data; }

        Size GetVertexDataSize() const { return m_vertexData.value().size; }
        Size GetGeometryDataSize() const { return m_geometryData.value().size; }
        Size GetFragmentDataSize() const { return m_fragmentData.value().size; }

        bool HasFragmentData() const { return m_fragmentData.has_value(); }
        bool HasGeometryData() const { return m_geometryData.has_value(); }
        bool HasVertexData() const { return m_vertexData.has_value(); }

        const char* GetFragmentEntryPoint() const { return  m_fragmentData.value().entryPoint.c_str(); }
        const char* GetVertexEntryPoint() const { return m_vertexData.value().entryPoint.c_str(); }
        const char* GetGeometryEntryPoint() const { return m_geometryData.value().entryPoint.c_str(); }

        void SetName(const char* name) override { this->name = name; };
        const char* GetName() override { return this->name.c_str(); }
        BlendState GetBlendState();
        CullMode GetCullMode();
        bool OverridesPipelineState() { return m_compilationObject.OverridePipelineState(); }
        VkShaderStageFlagBits GetShaderStages() override;

    protected:
        SPtr<Pipeline> OnCreatePipeline(PuduGraphics* gfx, RenderPass* renderPass) override;

    public:
        GPUResourceType Type() override { return GPUResourceType::Shader; }

    private:
        friend class PuduGraphics;

        std::filesystem::path m_shaderPath;

        std::optional<ShaderData> m_fragmentData;
        std::optional<ShaderData> m_vertexData;
        std::optional<ShaderData> m_geometryData;

        BlendState m_blendState;
        CullMode m_cullMode;
    };
}

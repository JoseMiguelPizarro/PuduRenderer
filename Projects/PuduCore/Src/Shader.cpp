#include "Shader.h"
#include "FrameGraph/RenderPass.h"
#include "PuduGraphics.h"

namespace Pudu
{
    void Shader::LoadFragmentData(const uint32_t* data, Size dataSize, const char* entryPoint)
    {
       if (dataSize <= 0)
       {
           LOG_WARNING("Trying to set fragment data with 0 size");
           return;
       }

        m_fragmentData = {.data = data, .size = dataSize, .entryPoint = entryPoint};
    }

    void Shader::LoadVertexData(const uint32_t* data, size_t dataSize, const char* entryPoint)
    {
        if (dataSize <= 0)
        {
            LOG_WARNING("Trying to set vertex data with 0 size");
            return;
        }

        m_vertexData = {.data = data, .size = dataSize, .entryPoint = entryPoint};
    }

    void Shader::LoadGeometryData(const u32* data, Size dataSize, const char* entryPoint)
    {
        if (dataSize <= 0)
        {
            LOG_WARNING("Trying to set geometry data with 0 size");
            return;
        }

        m_geometryData = {.data = data, .size = dataSize, .entryPoint = entryPoint};
    }

    BlendState Shader::GetBlendState()
    {
        return m_compilationObject.GetBlendState();
    }

    CullMode Shader::GetCullMode()
    {
        return m_compilationObject.GetCullMode();
    }

    SPtr<Pipeline> Shader::OnCreatePipeline(PuduGraphics* gfx, RenderPass* renderPass)
    {
        auto creationData = gfx->GetPipelineCreationData(this, renderPass);
        auto pipeline = gfx->CreateGraphicsPipeline(creationData);
        return pipeline.Get();
    }
}

#include "Shader.h"
#include "FrameGraph/RenderPass.h"
#include "PuduGraphics.h"

namespace Pudu
{
    void Shader::LoadFragmentData(const uint32_t* data, size_t dataSize, const char* entryPoint)
    {
        m_hasFragmentData = dataSize > 0;

        m_fragmentEntryPoint = entryPoint;
        if (dataSize > 0)
        {
            m_fragmentData = data;
        }
        m_fragmentDataSize = dataSize;
    }

    void Shader::LoadVertexData(const uint32_t* data, size_t dataSize, const char* entryPoint)
    {
        m_hasVertexData = dataSize > 0;
        m_vertexEntryPoint = entryPoint;

        if (dataSize > 0)
        {
            m_vertexData = data;
        }

        m_vertexDataSize = dataSize;
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

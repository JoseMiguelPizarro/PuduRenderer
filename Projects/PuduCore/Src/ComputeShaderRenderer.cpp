//
// Created by Hojaverde on 5/20/2025.
//

#include "ComputeShaderRenderer.h"

namespace Pudu
{
    ComputeShaderRenderer* ComputeShaderRenderer::SetShader(const SPtr<ComputeShader>& shader)
    {
        m_shader = shader;

        if (m_material != nullptr)
            m_material->SetShader(shader);

        return this;
    }

    ComputeShaderRenderer* ComputeShaderRenderer::SetMaterial(const SPtr<Material>& material)
    {
        m_material = material;
        return this;
    }
}

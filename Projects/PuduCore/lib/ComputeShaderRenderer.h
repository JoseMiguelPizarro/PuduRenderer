//
// Created by Hojaverde on 5/20/2025.
//

#pragma once
#include "ComputeShader.h"
#include "PuduCore.h"

namespace  Pudu
{
    class ComputeShaderRenderer
    {
        public:
        SPtr<ComputeShader> GetShader() const { return m_shader; }

        ComputeShaderRenderer* SetShader(const SPtr<ComputeShader>& shader);

        SPtr<Material> GetMaterial() const { return m_material; }

        ComputeShaderRenderer* SetMaterial(const SPtr<Material>& material);

    private:
        SPtr<ComputeShader> m_shader;
        SPtr<Material> m_material;
    };
}

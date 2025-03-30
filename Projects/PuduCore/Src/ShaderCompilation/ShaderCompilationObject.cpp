//
// Created by Hojaverde on 2/23/2025.
//
#include "PuduCore.h"
#include "ShaderCompilation/ShaderCompilationObject.h"

namespace Pudu
{
    constexpr f32 padding = 16.f;

    void ConstantBufferInfo::PushElement(Size elementSize)
    {
        this->size += static_cast<Size>(ceil(elementSize / padding) * padding);
    }

    void ShaderCompilationObject::AddKernel(const char* name, ShaderKernel& kernel)
    {
        m_kernelsByName[name] = kernel;
    }

    PushConstantInfo* ShaderCompilationObject::GetPushConstantsInfo()
    {
        return &m_pushConstantsInfo;
    }

    void ShaderCompilationObject::SetPushConstants(const std::vector<ConstantBufferInfo>& buffers)
    {
        m_pushConstants = buffers;

        for (const auto& buffer : m_pushConstants)
        {
            VkPushConstantRange pushConstant = {};
            pushConstant.offset = buffer.offset;
            pushConstant.size = buffer.size;
            pushConstant.stageFlags = buffer.shaderStages;
            m_pushConstantRanges.push_back(pushConstant);
        }
    }
}

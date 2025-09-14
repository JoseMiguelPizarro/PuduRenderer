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

    std::optional<ShaderKernel*> ShaderCompilationObject::GetKernel(const char* name)
    {
        if (m_kernelsByName.find(name) == m_kernelsByName.end())
            return std::nullopt;

        return &m_kernelsByName[name];
    }

    void ShaderCompilationObject::AddKernel(const char* name, ShaderKernel& kernel)
    {
        m_kernelsByName[name] = kernel;
    }

    PushConstantInfo* ShaderCompilationObject::GetPushConstantsInfo()
    {
        return &m_pushConstantsInfo;
    }

    void ShaderCompilationObject::SetPushConstants(const ConstantBufferInfo* buffers, Size count)
    {
        for (Size i = 0; i < count; i++)
        {
            m_pushConstants.push_back(buffers[i]);
            VkPushConstantRange pushConstant = {};
            pushConstant.offset = buffers[i].offset;
            pushConstant.size = buffers[i].size;
            pushConstant.stageFlags = buffers[i].shaderStages;
            m_pushConstantRanges.push_back(pushConstant);
        }
    }
}

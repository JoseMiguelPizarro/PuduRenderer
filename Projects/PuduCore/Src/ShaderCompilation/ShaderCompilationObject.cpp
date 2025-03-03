//
// Created by Hojaverde on 2/23/2025.
//
#include "PuduCore.h"
#include "ShaderCompilation/ShaderCompilationObject.h"

namespace Pudu
{
    void ConstantBufferInfo::PushElement(Pudu::size elementSize)
    {
        constexpr f32 padding = 16.f;
        this->size += static_cast<Pudu::size> (ceil(elementSize / padding)*padding);
    }

    void ShaderCompilationObject::AddKernel(const char* name, ShaderKernel& kernel) {
        m_kernelsByName[name] = kernel;
    }
}


//
// Created by Hojaverde on 2/23/2025.
//

#include "ShaderCompilation/ShaderCompilationObject.h"

namespace Pudu
{
    void ShaderCompilationObject::AddKernel(const char* name, ShaderKernel& kernel) {
        m_kernelsByName[name] = kernel;
    }
}



#pragma once
#include "PuduCore.h"

namespace Pudu
    {
struct ConstantBufferInfo
    {
        size setNumber = -1;
        size bindingIndex = -1;
        VkShaderStageFlags shaderStages;

        size offset = 0;
        size stride = 0;
        size size = 0;

        std::string name;
        void PushElement(size_t size);
    };
};

#pragma once
#include "PuduCore.h"

namespace Pudu
    {
struct ConstantBufferInfo
    {
        Size setNumber = -1;
        Size bindingIndex = -1;
        VkShaderStageFlags shaderStages;

        Size offset = 0;
        Size stride = 0;
        Size size = 0;

        std::string name;
        void PushElement(Size size);
    };
};
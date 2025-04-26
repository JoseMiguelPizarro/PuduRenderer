#pragma once
#include "PuduCore.h"
#include <vulkan/vulkan.h>

namespace Pudu
{
    enum TextureSampleCount
    {
        One = 0x1,
        Two = 0x2,
        Four = 0x4,
        Eight = 0x8,
        Sixteen = 0x10,
        ThirtyTwo = 0x20,
        SixtyFour = 0x40,
    };

    

    struct AntialiasingSettings
    {
        TextureSampleCount sampleCount = TextureSampleCount::One;
    };
}

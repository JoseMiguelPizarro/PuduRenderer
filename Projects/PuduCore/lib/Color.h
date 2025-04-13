#pragma once
#include "PuduCore.h"
namespace Pudu {
struct Color
  {
union {
    struct {
        float r, g, b, a;
    };
    vec4 asVector;
    float data[4];
};

operator vec4&()
{
    return asVector;
}

operator const vec4&() const
{
    return asVector;
}
  };
}


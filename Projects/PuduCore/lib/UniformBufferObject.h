#pragma once
#include "hlsl++.h"

using namespace hlslpp;

struct UniformBufferObject {
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 ProjectionMatrix;
};
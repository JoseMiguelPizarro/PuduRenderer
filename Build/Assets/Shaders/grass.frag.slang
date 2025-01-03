#include "lib/Lighting.hlsl"
#include "lib/Shadowmap.slang"

import "lib/grass";

ParameterBlock<GrassData> Data;

[shader("fragment")]
float4 fragmentMain(GrassVertexOutput input)
    : SV_Target
{
    return input.color;
}

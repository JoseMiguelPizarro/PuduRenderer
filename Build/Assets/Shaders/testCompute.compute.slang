#include "lib/grass.slang"

ParameterBlock<GrassData> Data;

[shader("compute")]
[numthreads(32, 32, 1)]
void main(uint3 threadId: SV_DispatchThreadID)
{
    float2 resolution = float2(1024, 1024);
    float2 uv = threadId.xy / resolution;
    Data.GrassPos[threadId.x] = { float2(1, 0) };
}

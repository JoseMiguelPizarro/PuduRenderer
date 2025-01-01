#include "lib/grass.slang"

ParameterBlock<GrassData> Data;

[shader("compute")]
[numthreads(32, 32, 1)]
void main(uint3 threadId: SV_DispatchThreadID)
{
    uint dimensions;
    uint stride;

    Data.GrassPos.GetDimensions(dimensions, stride);
    var sqrtDimension = sqrt(dimensions);
    var size = sqrtDimension * 0.5;

    uint id = (uint)sqrtDimension * threadId.y + threadId.x;
    Data.GrassPos[id] = { (float2(threadId.x, threadId.y) - float2(size, size)) * 0.2 };
}

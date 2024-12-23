RWTexture2D screen;

[shader("compute")]
[numthreads(32, 32, 1)]
void main(uint3 threadId: SV_DispatchThreadID)
{
    float2 resolution = float2(1024, 1024);
    float2 uv = threadId.xy / resolution;
    screen[threadId.x + threadId.y * 1024] = float4(1, 0, 0, 0);
}

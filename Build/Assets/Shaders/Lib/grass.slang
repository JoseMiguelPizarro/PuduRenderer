struct Grass
{
    float2 pos;
}

struct GrassData
{
    RWStructuredBuffer<Grass> GrassPos;
};

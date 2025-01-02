struct Grass
{
    float2 pos;
}

struct GrassData
{
    RWStructuredBuffer<Grass> GrassPos;
    Sampler2D shadowMap;
    uint stripes;
};

struct GrassVertexOutput
{
    float4 positionCS : SV_Position;
    float4 color : COLOR;
}

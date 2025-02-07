struct Grass
{
    float2 pos;
}

struct GrassData
{
    RWStructuredBuffer<Grass> GrassPos;
    Sampler2D shadowMap;
};

struct GrassVertexOutput
{
    float4 PosCS : SV_Position;
    float4 posWS : POSITION;
    float4 color : COLOR;
    float4 shadowCoords : TEXCOORD;
}

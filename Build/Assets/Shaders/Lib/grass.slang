module GrassModule;

public struct Grass
{
    public float2 pos;
}

public struct GrassData
{
    public RWStructuredBuffer<Grass> GrassPos;
    public Sampler2D shadowMap;
};

public struct GrassVertexOutput
{
    public float4 positionCS : SV_Position;
    public float4 posWS : POSITION;
    public float4 color : COLOR;
    public float4 shadowCoords : TEXCOORD;
}

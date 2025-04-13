#ifndef DefaultVertexInput_h
    #define DefaultVertexInput_h


struct VSOut
{
    float4 PosCS: SV_POSITION;
    float4 PosWS:POSITION0;
    float4 PosOS:POSITION1;
    float3 viewDir:POSITION2;
    float4 Color: COLOR;
    float4 TexCoord:TEXCOORD;
    float4 Normal:NORMAL;
    float4 Tangent:TANGENT;
    float4 ShadowCoords:TEXCOORD1;

};

struct VertexInput
{
    float3 Position:POSITION;
    float3 Color: COLOR;
    float2 TexCoord: TEXCOORD;
    float3 Normal: NORMAL;
    float4 Tangent:TANGENT;
};

#endif
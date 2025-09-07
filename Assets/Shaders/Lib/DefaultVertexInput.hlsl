

#ifndef DefaultVertexInput_h
    #define DefaultVertexInput_h
import PuduCoreModule;

struct VSOut
{
    float4 PosCS: SV_POSITION;
    float4 PosWS:POSITION0;
    float4 PosOS:POSITION1;
    float4 Color: COLOR;
    float3 viewDir:POSITION2;
    float4 TexCoord:TEXCOORD;
    float4 Normal:NORMAL;
    float4 Tangent:TANGENT;
    float4 ShadowCoords:TEXCOORD1;

};

struct VertexInput
{
 	[POSITION] 	float3 Position;
    [COLOR] 	float4 Color;
    [TEXCOORD0] float2 TexCoord;
    [NORMAL] 	float3 Normal;
    [TANGENT] 	float4 Tangent;
};

#endif
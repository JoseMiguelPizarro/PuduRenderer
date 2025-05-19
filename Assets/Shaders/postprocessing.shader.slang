#include "Lib/DefaultVertexInput.hlsl"

import PuduGraphicsModule;

struct PostProcessingData
{
    Sampler2D colorTex;
}

ParameterBlock<PostProcessingData> Data;

static float4 pos[] = {
    float4(-1, -1, 0, 0),
    float4(-1, 1, 0, 0),
    float4(1, 1, 0, 0),
    float4(1, -1, 0, 0)
};

[shader("vertex")]
VSOut vertexMain(VertexInput in, uint vertexId: SV_VertexID)
{
    VSOut output = (VSOut)0.0;
    output.PosCS = float4(pos[vertexId].xyz, 1);
    output.TexCoord = float4(in.TexCoord, 0, 0);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
    Sampler2D tex = Data.colorTex;

    float2 baseUV = input.TexCoord.xy;
    float mask = length(baseUV - 0.5);
    float vignette =saturate( smoothstep(0, .8, 1 - mask));
    mask *= 1.5;
    mask = saturate(mask);

    float chromatic = 0.15;
    float depth =LinearizeDepth( GLOBALS.depthBuffer.Sample(baseUV).r);


    float r = tex.Sample(lerp(baseUV, baseUV, mask)).x;
    float g = tex.Sample(lerp(baseUV, baseUV + float2(0.005, 0.005)*chromatic, mask)).y;
    float b = tex.Sample(lerp(baseUV, baseUV + float2(0.01, 0.01)*chromatic, mask)).z;
    float d =  saturate(1. - exp(-depth/40));

    return float4(r,g,b,0) * vignette;
}

#include "Lib/DefaultVertexInput.hlsl"


import PuduCoreModule;
import "Lib/PuduDebugModule";
import PuduGraphicsModule;

struct PostProcessingData
{
    Sampler2D colorTex;
    float exposure;
};

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
    Sampler2D screenColorTex = Data.colorTex;
    float2 baseUV = input.TexCoord.xy;
    float4 c = 0;

    if (DEBUG_SETTINGS.toneMapping)
    {
        float mask = length(baseUV - 0.5);
        float vignette = saturate(smoothstep(0, .8, 1 - mask));
        mask *= 1.5;
        mask = saturate(mask);

        float chromatic = 0.15;
        float depth = LinearizeDepth(GLOBALS.depthBuffer.Sample(baseUV).r);

        // TONEMAP
        float exposure = Data.exposure;

        float r = ToneMap_Reinhard(screenColorTex.Sample(lerp(baseUV, baseUV, mask)), exposure).x;
        float g = ToneMap_Reinhard(screenColorTex.Sample(lerp(baseUV, baseUV + float2(0.005, 0.005) * chromatic, mask)), exposure).y;
        float b = ToneMap_Reinhard(screenColorTex.Sample(lerp(baseUV, baseUV + float2(0.01, 0.01) * chromatic, mask)), exposure).z;

        float d = saturate(1. - exp(-depth / 40));

        c = float4(r, g, b, 0) * vignette;
    }
    else
    {
        c = screenColorTex.Sample(baseUV);
    }

    // GAMMA CORRECT
    c.rgb = GammaCorrect(c.rgb, GLOBALS.gamma);
    return c;
}
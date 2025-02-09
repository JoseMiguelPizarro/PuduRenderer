#include "Lib/DefaultVertexInput.hlsl"
#include "lib/Shadowmap.slang"

import PuduGraphics;


struct StandardMaterial
{
    float4 color;
}


[shader("vertex")]
VSOut vertexMain(VertexInput input, uint vertexId: SV_VertexID)
{
    VSOut output = (VSOut)0.0;
    output.PosCS =  mul(UBO.proj, mul(UBO.view, mul(UBO.model, float4(input.Position, 1.0))));
    output.PosWS =  mul(UBO.model, float4(input.Position, 1.0));
    output.TexCoord = float4(input.TexCoord, 0, 0);
    output.Normal = float4(input.Normal,0.);
    output.viewDir = output.PosWS.xyz - GLOBALS.constants.cameraPosWS.xyz;

    LightBuffer lightingBuffer = GLOBALS.lightingBuffer;
   output.ShadowCoords = GetShadowCoord(lightingBuffer.shadowMatrix, lightingBuffer.lightMatrix, output.PosWS);

    return output;
}

static const float maxDist = 30;
static const float step = 0.05;
static const float thickness = 1.5;

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
    float2 screenUv = GetPixelScreenPosition(input.PosCS);
    float3 normal = normalize(input.Normal).xyz;

    float3 viewDir = normalize(input.viewDir);

    float3 rayDir = reflect(viewDir, normal);

    float traversedDistance = 0;
    float3 rayPos = input.PosWS.xyz;

    float3 c = 0.;
    while(traversedDistance<maxDist)
    {
        rayPos += rayDir*step;

        float4 posProjected = mul(UBO.proj,mul(UBO.view, float4(rayPos.xyz,1.)));
        float rayDepth = posProjected.z/posProjected.w;
        float2 posUv = 0.5 + 0.5 * (posProjected.xy / posProjected.w).xy;
        posUv.y = 1. - posUv.y;

        if(posUv.x<0 ||posUv.x>1 ||posUv.y <0 || posUv.y>1)
        {
            break;
        }

       // float rayDepth = distance(rayPos,GLOBALS.constants.cameraPosWS.xyz);
        float pixelDepth = GLOBALS.depthBuffer.Sample(posUv).x;

        if((rayDepth - pixelDepth)<thickness)
        {
            c = GLOBALS.normalBuffer.Sample(posUv).xyz;
            break;
        }

        traversedDistance+=step;
    }

    float shadow = saturate(FilterPCF(input.ShadowCoords / input.ShadowCoords.w, GLOBALS.shadowMap) + 0.5);

    c = (float3(0.3,.5,0.8) + c*0.5)*(1 - shadow*0.5);


    return float4(c,1.);
}
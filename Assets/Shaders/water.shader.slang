#include "Lib/DefaultVertexInput.hlsl"
#include "lib/Shadowmap.slang"

import PuduGraphicsModule;


struct WaterMaterial
{
    Sampler2D normalTex;
    SamplerCube skyTex;
    float intensity = 1.;
}


ParameterBlock<WaterMaterial> material;

[shader("vertex")]
VSOut vertexMain(VertexInput input, uint vertexId: SV_VertexID)
{
    VSOut output = (VSOut)0.0;
    output.PosWS =  mul(UBO.model, float4(input.Position, 1.0));
    output.PosCS =  mul(GetProjectionMatrix(),mul(GetViewMatrix(), output.PosWS));
    output.TexCoord = float4(input.TexCoord, 0, 0);
    output.Normal = float4(input.Normal,0.);
    output.viewDir = output.PosWS.xyz - GLOBALS.constants.cameraPosWS.xyz;

    LightBuffer lightingBuffer = GLOBALS.lightingBuffer;
    output.ShadowCoords = GetShadowCoord(lightingBuffer.shadowMatrix, lightingBuffer.lightMatrix, output.PosWS);

    return output;
}

static const float maxDist = 30;
static const float step = 0.1;
static const float thickness = 0.001;

[shader("fragment")]
float4 fragmentMain(VSOut input)
    : SV_TARGET
{
    var time = GLOBALS.constants.time*.5;
    float2 screenUv = GetPixelScreenPosition(input.PosCS);
    float3 normal =material.normalTex.Sample(input.PosWS.xz*0.7 + time*0.03).xyz;
    float3 n2 = material.normalTex.Sample(input.PosWS.xz*0.2 + time*0.06).xyz;

    normal = lerp(normal,n2,0.5);

    normal = normalize(lerp(normal,float3(0,1,0),0.95));

    var vtangent = float3(1,0,0);
    var vbitangent = float3(0,0,1);
    var vnormal = float3(0,1,0);


    normal = normalize(normal);
    normal = normalize(TangentToWorld(normal, vtangent,vbitangent,vnormal));
  //  normal = float3(0,1,0);

    float3 viewDir = normalize(input.viewDir);

    float3 rayDir = normalize(reflect(viewDir, normal));

    float traversedDistance = 0;
    float3 rayPos = input.PosWS.xyz;

    float3 c = 0.;

    float hitDepth = 0;

    traversedDistance = dot(rayDir, -viewDir)>0.? maxDist:0;

    while(traversedDistance<maxDist)
    {
        rayPos += rayDir*step;

        float4 posProjected = mul(GetProjectionMatrix(),mul(GetViewMatrix(), float4(rayPos.xyz,1.)));
        float rayDepth = posProjected.z/posProjected.w;

        //Convert raypos to ScreenSpace uvs. Perspective divide transform the position to NDC space
        float2 posUv = 0.5 + 0.5 * (posProjected.xy / posProjected.w).xy;

        if(posUv.x<0 ||posUv.x>1 ||posUv.y <0 || posUv.y>1)
        {
            traversedDistance = maxDist;
            break;
        }

        float pixelDepth = GLOBALS.depthBuffer.Sample(posUv).x;

        if((rayDepth - pixelDepth) > thickness)
        {
            c = GLOBALS.colorBuffer.Sample(posUv).xyz;
            break;
        }

        traversedDistance+=step;
    }

    if(traversedDistance>=maxDist)
    {
      c = material.skyTex.Sample(rayDir).xyz;
    }

    float shadow = saturate(FilterPCF(input.ShadowCoords / input.ShadowCoords.w, GLOBALS.shadowMap) + 0.5);

    c = (c)*(shadow) * .85;

    return float4(c,1.);
}
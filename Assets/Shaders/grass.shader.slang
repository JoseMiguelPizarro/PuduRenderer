import PuduGraphics;

#include "Lib/grass.slang"
#include "Lib/Shadowmap.slang"

#include "lib/lygia/generative/cnoise.hlsl"
#include "lib/lygia/generative/random.hlsl"

ParameterBlock<GrassData> Data;

static float3 vertices[] = {
    float3(0., 0, 0),
    float3(0.5, 1, 0),
    float3(1, 0, 0)
};

[shader("vertex")]
GrassVertexOutput vertexMain(uint id: SV_InstanceID, uint vertexId: SV_VertexID)
{
    GrassVertexOutput output = {};
    var grass = Data.GrassPos[id];
    var randomPos = random3(id);
    var pos = grass.pos.xyz;

    var stripeId = vertexId / 6;
    var triangleId = vertexId / 3 % 2;
    var indexId = vertexId % 3;

    float a = (indexId + triangleId) % 3 / 2;
    float b = ceil(indexId / 2.);

    float3 vertexPos;
    if (stripeId >= 6)
    {
        vertexPos = float3(vertices[indexId].xy, 0) + float3(0, stripeId, 0);
    }
    else
    {
        vertexPos = float3(a * (1 - triangleId) + b * triangleId, a * triangleId + b * (1 - triangleId) + stripeId, 0);
    }

    float3 forward = float3(UBO.view[0][2], UBO.view[1][2], UBO.view[2][2]);
    float3 right = float3(UBO.view[0][0], UBO.view[1][0], UBO.view[2][0]);
    float3 up = float3(UBO.view[0][1], UBO.view[1][1], UBO.view[2][1]);

    float4x4 billBoard = {
        float4(right, 0),
        float4(up, 0),
        float4(forward, 0),
        float4(0, 0, 0, 1)
    };

    float2 size = float2(.1, (0.13 + randomPos.x*0.02)) * grass.pos.w;
    vertexPos *= float3(size.x, size.y / 6, 1);
    vertexPos = mul(billBoard, float4(vertexPos, 1)).xyz;

    var vgradient = vertexPos.y / size.y;
    float windIntensity = 0.01 + randomPos.y*0.02;
    float speed = 0.2;
    float2 wind = cnoise(pos * .8 + UBO.time * .6 * speed) * windIntensity + pow(cnoise(pos * 0.5 + UBO.time * 2 * speed)  * 2, 3)* windIntensity;
    wind *= vgradient;

    float3 vertex = { vertexPos.x + wind.x, vertexPos.y, wind.y + vertexPos.z };

    float grassLength = length(vertexPos);
    float newLength = length(vertex);
    float ratio = grassLength / newLength;
    pos += vertex;

    LightBuffer lightingBuffer = GLOBALS.lightingBuffer;
    output.shadowCoords = GetShadowCoord(GLOBALS.lightingBuffer.shadowMatrix, GLOBALS.lightingBuffer.lightMatrix, float4(pos, 1));
    output.PosCS = mul(UBO.proj, mul(UBO.view, float4(pos, 1)));

    output.color = lerp(float4(.5, .6, .7, 1.), float4(.6, .8, .5, 1.), vgradient + length(wind)*5.);

    return output;
}

[shader("fragment")]
float4 fragmentMain(GrassVertexOutput input)
    : SV_Target
{
    float4 col = input.color;
    float3 viewPos = float3(UBO.view._14, UBO.view._24, UBO.view._34);
    float d = 1. - saturate(distance(input.posWS.xyz, viewPos) / 80.f);
    float shadow = saturate(FilterPCF(input.shadowCoords / input.shadowCoords.w, GLOBALS.shadowMap) + 0.5);

    col = lerp(col, float4(0.2, 0.2, 0.3, 1.0), 1 - shadow);
    return lerp(float4(0.9, .4, 0.6, 0.), col, d);
}


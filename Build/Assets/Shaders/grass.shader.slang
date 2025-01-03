import PuduGraphics;
import "Lib/grass";
import "Lib/Shadowmap";

#include "lib/lygia/generative/cnoise.hlsl"
#include "lib/lygia/generative/random.hlsl"

[[vk::push_constant]]
ConstantBuffer<UniformBufferObject> ubo;

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
    var randomPos = random2(grass.pos) * .2;
    var pos = float3(grass.pos.x + randomPos.x, 0, grass.pos.y + randomPos.y);

    var stripeId = vertexId / 6;
    var triangleId = vertexId / 3 % 2;
    var indexId = vertexId % 3;

    float a = (indexId + triangleId) % 3 / 2;
    float b = ceil(indexId / 2.);

    float2 vertexPos;
    if (stripeId >= 6)
    {
        vertexPos = vertices[indexId].xy + float2(0, stripeId);
    }
    else
    {
        vertexPos = float2(a * (1 - triangleId) + b * triangleId, a * triangleId + b * (1 - triangleId) + stripeId);
    }

    float2 size = { .2, .5 };
    vertexPos.x -= randomPos.x * 0.1;
    vertexPos *= float2(size.x, size.y / 6);

    var vgradient = vertexPos.y / size.y;
    float windIntensity = 0.1 + randomPos.y;
    float2 wind = cnoise(pos * .3 + ubo.time * .6) * windIntensity + pow(cnoise(pos * 0.1 + ubo.time * 2) * windIntensity * 2, 3);
    wind *= vgradient;

    float3 vertex = { vertexPos.x + wind.x, vertexPos.y, wind.y };

    float grassLength = length(vertexPos);
    float newLength = length(vertex);
    float ratio = grassLength / newLength;
    pos += vertex * ratio;

    output.positionCS = mul(ubo.proj, mul(ubo.view, float4(pos, 1)));

    output.color = lerp(float4(.5, .6, .7, 1.), float4(.6, .8, .5, 1.), vgradient + length(wind));

    return output;
}

[shader("fragment")]
float4 fragmentMain(GrassVertexOutput input)
    : SV_Target
{
    return input.color;
}


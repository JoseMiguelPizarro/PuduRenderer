import "../PuduGraphics";

// Column major
static const float4x4 biasMat = float4x4(
    { 0.5, 0, 0, 0.5 },
    { 0, 0.5, 0, 0.5 },
    { 0, 0, 1.0, 0 },
    { 0, 0, 0, 1.0 });

float TextureProj(float4 shadowCoord, float2 off, Sampler2D shadowMap)
{
    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
    {
        Sampler2D tex = shadowMap;
        float dist = tex.Sample(shadowCoord.xy + off).x;
        if (shadowCoord.w > 0.0 && dist < shadowCoord.z)
        {
            shadow = 0.0f;
        }
    }
    return shadow;
}

float FilterPCF(float4 sc, Sampler2D shadowMap)
{
    int2 texDim;
    shadowMap.GetDimensions(texDim.x, texDim.y);
    float scale = 1.5;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    int range = 1;

    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            shadowFactor += TextureProj(sc, float2(dx * x, dy * y), shadowMap);
            count++;
        }
    }
    return shadowFactor / count;
}

float4 GetShadowCoord(float4x4 shadowMatrix, float4x4 lightMatrix, float4 posWS)
{
    return mul(biasMat, mul(shadowMatrix, mul(lightMatrix, posWS)));
}

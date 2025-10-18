module PuduShadowModule;

// Column major. Convert clipspace shadowcoord to 0-1 uv space
static const float4x4 biasMat = float4x4(
    { 0.5, 0, 0, 0.5 },
    { 0, 0.5, 0, 0.5 },
    { 0, 0, 1.0, 0 },
    { 0, 0, 0, 1.0 });

float TextureProj(float4 shadowCoord, float2 off, Sampler2DShadow shadowMap)
{
        float shadow = 1.0;

        float dist = shadowMap.Sample(shadowCoord.xy + off).x;
        if (shadowCoord.w>0 && dist < shadowCoord.z)
        {
            shadow = 0.0f;
        }
    return shadow;
}

public struct OmnidirectionalShadowSampleData
{
    public uint lightIndex;
    public Sampler2DShadow shadowMap;
    public float3 positionWS;
    public float3 lightVector;
    public float4x4 shadowMatrix[4];
    public uint2 lightDimension;
    public float centerOffset;
};

const static float3 faceVectors[4] =
{
  float3(0.0, -0.57735026, -0.81649661), //Green face
  float3(0.0, -0.57735026, 0.81649661), //Yellow face
  float3(-0.81649661, 0.57735026, 0.0),// Blue face
  float3(0.81649661, 0.57735026, 0.0)// Red face
};



uint GetFaceIndex(float3 dir)
{
  float4x3 faceMatrix;
  float dA = dot(dir, faceVectors[0]);
  float dB = dot(dir, faceVectors[1]);
  float dC = dot(dir, faceVectors[2]);
  float dD = dot(dir, faceVectors[3]);

  uint index = 0;
  float currentMax = dA;
  if(dB>currentMax)
  {
    index = 1;
    currentMax = dB;
  }
  if(dC>currentMax)
  {
    index = 2;
    currentMax = dC;
  }
  if(dD > currentMax)
  {
    index = 3;
    currentMax = dD;
  }

  return index;
}

public float SampleOmnidirectionalLightShadow(OmnidirectionalShadowSampleData sampleData)
{
    uint faceIndex = GetFaceIndex(sampleData.lightVector);
    float4 shadowCoord = mul(sampleData.shadowMatrix[faceIndex], float4(sampleData.positionWS,1));

     shadowCoord /= shadowCoord.w;

     shadowCoord = mul(biasMat, shadowCoord);

     float shadow = 1.0;

    float dist = sampleData.shadowMap.Sample(shadowCoord.xy).x;
    if (shadowCoord.w>0 && dist < shadowCoord.z)
    {
        shadow = 0.0f;
    }

    return shadow;
}

public float FilterPCF(float4 sc, Sampler2DShadow shadowMap)
{
    sc /=sc.w;

    int2 texDim;
    shadowMap.GetDimensions(texDim.x, texDim.y);
    float scale = 1;
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

public float4 GetShadowCoord(float4x4 shadowMatrix, float4x4 lightMatrix, float4 posWS)
{
    return mul(biasMat, mul(shadowMatrix, mul(lightMatrix, posWS)));
}

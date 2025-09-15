import PuduCoreModule;
import PuduGraphicsModule;


struct VertexInput
{
 	[POSITION] float3 Position;
};

struct GSOutput
{
    float4 pos   : SV_POSITION;
    float clipDistance[3]: SV_ClipDistance;
};

struct VSOutput{
    float4 position;
};

static const float3 planeNormals[12] =
{
  float3(0.00000000, -0.03477280, 0.99939519),
  float3(-0.47510946, -0.70667917, 0.52428567),
  float3(0.47510946, -0.70667917, 0.52428567),
  float3(0.00000000, -0.03477280, -0.99939519),
  float3(0.47510946, -0.70667917, -0.52428567),
  float3(-0.47510946, -0.70667917, -0.52428567),
  float3(-0.52428567, 0.70667917, -0.47510946),
  float3(-0.52428567, 0.70667917, 0.47510946),
  float3(-0.99939519, 0.03477280, 0.00000000),
  float3(0.52428567, 0.70667917, -0.47510946),
  float3(0.99939519, 0.03477280, 0.00000000),
  float3(0.52428567, 0.70667917, 0.47510946)
};

    void GetTextureSpaceMatrix(float2 p, float size, inout float4x4[4] tm)
    {
        float s = size; //Size
        tm[0] = float4x4({s, 0, 0, 0}, {0, s / 2, 0, 0}, {0, 0, 1, 0}, {p.x, p.y - s / 2, 0, 1});
        tm[1] = float4x4({s / 2, 0, 0, 0}, {0, s, 0, 0}, {0, 0, 1, 0}, {p.x + s / 2, p.y, 0, 1});
        tm[2]= float4x4({s, 0, 0, 0}, {0, s / 2, 0, 0}, {0, 0, 1, 0}, {p.x, p.y + s / 2, 0, 1});
        tm[3]= float4x4({s / 2, 0, 0, 0}, {0, s, 0, 0}, {0, 0, 1, 0}, {p.x - s / 2, p.y, 0, 1});
    }

float GetClipDist(in float3 lightPos, in uint planeIndex, float3 vertexPosition)
{
    float3 normal = planeNormals[planeIndex]; // clipping plane normal
    return (dot(vertexPosition, normal) + dot(-normal, lightPos));
}

struct OmnidirectionalShadowmapData
{
        float4 lightCount;
        float4x4 shadowMatrix[128]; //FOR NOW HARD CODED TO 128. Packed in sets of 4 per each light
};

struct Data{
ConstantBuffer<OmnidirectionalShadowmapData> data;
};

ParameterBlock<Data> LightData;


[shader("vertex")]
VSOutput vertexMain(VertexInput input)
{
    VSOutput output = {0};
    output.position =mul(UBO.model, float4(input.Position,1.0f));

    return output;
};


[shader("geometry")]
[maxvertexcount(12)]
void geometryMain(triangle VSOutput input[3], inout TriangleStream<GSOutput> triStream)
{
    uint lightIndex  = UBO.custom.x;
    float3 lightPosition = LIGHTING.lights.pointLight[lightIndex].positionAndRange.xyz;

 // back-face culling
 float3 normal = cross(input[2].position.xyz-input[0].position.xyz, input[0].position.xyz - input[1].position.xyz);
 float3 view = lightPosition-input[0].position.xyz;

 if(dot(normal, view) < 0.0f)
   return;

   float4x4 tm[4];
    GetTextureSpaceMatrix(float2(0,0),0.0625,tm);

 for(uint faceIndex=0; faceIndex<4; faceIndex++)
 {
   uint inside = 0;
   float clipDistances[9];
   for(uint sideIndex=0; sideIndex<3; sideIndex++)
   {
     const uint planeIndex = (faceIndex*3)+sideIndex;
     const uint bit = 1 << sideIndex;
     for(uint vertexIndex=0; vertexIndex<3; vertexIndex++)
     {
       uint clipDistanceIndex = sideIndex*3+vertexIndex;
       clipDistances[clipDistanceIndex] = GetClipDist(lightPosition, planeIndex, input[vertexIndex].position.xyz);
       inside |= (clipDistances[clipDistanceIndex] > 0.001) ? bit : 0;
     }
   }

   if(inside == 0x7)
   {
        const float4x4 shadowViewProjTexMatrix = LightData.data.shadowMatrix[lightIndex + faceIndex];
        GSOutput output;
        float4x4 texM = tm[faceIndex];

         for(uint vertexIndex=0; vertexIndex<3; vertexIndex++)
         {
            output.pos = mul(shadowViewProjTexMatrix, input[vertexIndex].position);

            output.clipDistance[0] = clipDistances[vertexIndex];
            output.clipDistance[1] = clipDistances[3+vertexIndex];
            output.clipDistance[2] = clipDistances[6+vertexIndex];

            triStream.Append(output);
         }
     triStream.RestartStrip();
   }
 }
}

[shader("fragment")]
float4 fragmentMain(float4 input)
    : SV_Target
{
    return 1;
}

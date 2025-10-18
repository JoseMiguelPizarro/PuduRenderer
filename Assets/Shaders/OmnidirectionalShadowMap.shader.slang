import PuduCoreModule;
import PuduGraphicsModule;

#define LOOK_AT_RIGHT_HANDED
#include "lib/lygia/space/lookat.hlsl"

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
    float4 position: SV_POSITION;
};


const static float3 faceVectors[4] =
{
  float3(0.0, -0.57735026, -0.81649661), //Green face
  float3(0.0, -0.57735026, 0.81649661), //Yellow face
  float3(-0.81649661, 0.57735026, 0.0),// Blue face
  float3(0.81649661, 0.57735026, 0.0)// Red face
};

//Aspect ratio: Width/Height
	static float4x4 PerspectiveMatrix(float vertical_fov, float aspect_ratio, float n, float f)
	{
		float fov_rad = vertical_fov * 2.0f * PI / 360.0f;
		float focal_length = 1.0f / tan(fov_rad / 2.0f);

		float x = focal_length * aspect_ratio;
		float A = f / (f - n);
		float B = -n * A;

		float4x4 pm =
		float4x4(
			{aspect_ratio * focal_length, 0.0f, 0.0f, 0.0f},
			{0.0f, -focal_length, 0.0f, 0.0f}, //Multiply by -1 to remap to vk ndc space
			{0.0f, 0.0f, A, 1},
			{0.0f, 0.0f, B, 0.0f});

		return transpose(pm);
	}

	static float4x4 PerspectiveMatrixFOV(float vertical_fov, float horizontal_fov, float n, float f)
    	{
    		float v_fov_rad = vertical_fov * 2.0f * PI / 360.0f;
    		float h_fov_rad = horizontal_fov * 2.0f * PI / 360.0f;

    		float focal_length_v = 1.0f / tan(v_fov_rad / 2.0f);
    		float focal_length_h = 1.0f / tan(h_fov_rad / 2.0f);

    		float A = f / (f - n);
    		float B = -n * A;

    		float4x4 pm =
    		{
    			focal_length_h, 0.0f, 0.0f, 0.0f,
    			0.0f, -focal_length_v, 0.0f, 0.0f, //Multiply by -1 to remap to vk ndc space
    			0.0f, 0.0f, A, 1,
    			0.0f, 0.0f, B, 0.0f};

    		return transpose(pm);
    	}

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

struct Data{
ConstantBuffer<OmnidirectionalShadowmapData> data;
};

ParameterBlock<Data> LightData;




[shader("vertex")]
VSOutput vertexMain(VertexInput input)
{
    float3 forward = faceVectors[0];
    float3 up = cross(forward,{-1,0,0});
    float3x3 view3x3 = lookAt(forward,up);

    float4x4 view = {
        float4(view3x3[0], 0),
        float4(view3x3[1], 0),
        float4(view3x3[2], 0),
        float4(0, 0, 0, 1)
    };
    float4x4 viewProj = GLOBALS.constants.viewProjection;
    float fov_x = 143.98570868;
    float fov_y = 125.26438968;
    float4x4 proj =PerspectiveMatrixFOV(fov_y,fov_x,1,100);
    VSOutput output = {0};
    //view = GLOBALS.constants.view;
    output.position = mul(proj,mul(view, mul(UBO.model, float4(input.Position,1.0f))));

    return output;
};

//[shader("geometry")]
//[maxvertexcount(12)]
//void geometryMain(triangle VSOutput input[3], inout TriangleStream<GSOutput> triStream)
//{
//    uint lightIndex  = 0;
//    float3 lightPosition = LIGHTING.lights.pointLight[lightIndex].positionAndRange.xyz;
//
// // back-face culling
// float3 normal = -cross(input[2].position.xyz-input[0].position.xyz, input[0].position.xyz - input[1].position.xyz);
// float3 view = lightPosition-input[0].position.xyz;
//
// if(dot(normal, view) < 0.0f)
//   return;
//
//   float4x4 tm[4];
//   GetTextureSpaceMatrix(float2(0,0),1,tm);
//
// for(uint faceIndex=0; faceIndex<4; faceIndex++)
// {
//   uint inside = 0;
//   float clipDistances[9];
//   for(uint sideIndex=0; sideIndex<3; sideIndex++)
//   {
//     const uint planeIndex = (faceIndex*3)+sideIndex;
//     const uint bit = 1 << sideIndex;
//     for(uint vertexIndex=0; vertexIndex<3; vertexIndex++)
//     {
//       uint clipDistanceIndex = sideIndex*3+vertexIndex;
//       clipDistances[clipDistanceIndex] = GetClipDist(lightPosition, planeIndex, input[vertexIndex].position.xyz);
//       inside |= (clipDistances[clipDistanceIndex] > 0.001) ? bit : 0;
//     }
//   }
//
//   if(inside==0x7)
//   {
//        const float4x4 shadowViewProjTexMatrix = LightData.data.shadowMatrix[lightIndex*4 + faceIndex];
//        GSOutput output;
//        float4x4 texM = tm[faceIndex];
//
//         for(uint vertexIndex=0; vertexIndex<3; vertexIndex++)
//         {
//            output.pos =mul(shadowViewProjTexMatrix,float4(input[vertexIndex].position.xyz,1));
//
//            float clipOffset = 0;
//            output.clipDistance[0] = clipDistances[vertexIndex] + clipOffset;
//            output.clipDistance[1] = clipDistances[3+vertexIndex] + clipOffset;
//            output.clipDistance[2] = clipDistances[6+vertexIndex] + clipOffset;
//
//            triStream.Append(output);
//         }
//     triStream.RestartStrip();
//   }
// }
//}

[shader("fragment")]
float4 fragmentMain(float4 input)
    : SV_Target
{
    return 1.0;
}

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

//TODO: MAKE PLANES NORMALS MATCH THE FACE ORDER
static const float3 planeNormals[12] =
{
  //Green Face
  float3(0.00000000, 0, -1),
  float3(0.47510946, -0.707107, -0.5),
  float3(-0.5, -0.707107, -0.4999),
  //Yellow Face
  float3(0.00000000, 0, 1.),
  float3(0.5, -0.707107, 0.5),
  float3(-0.5, -0.707107, 0.49999),
  //Blue Face
  float3(-0.5, 0.707107, 0.5),
  float3(-1, 0, 0),
  float3(-0.5, 0.707107, -0.5),
  //Red Face
  float3(0.5, 0.707107, -0.49999),
  float3(1., 1.8841e-6, 0.00000000),
  float3(0.5, 0.707107, 0.49999)
};


    void GetTextureSpaceMatrix(float2 p, float size, inout float4x4[4] tm)
    {
        float s = size; //Size
        tm[0] = float4x4({s, 0, 0, 0}, {0, s / 2, 0, 0}, {0, 0, 1, 0}, {p.x, p.y - s / 2, 0, 1});
        tm[1] = float4x4({s / 2, 0, 0, 0}, {0, s, 0, 0}, {0, 0, 1, 0}, {p.x + s / 2, p.y, 0, 1});
        tm[2]= float4x4({s, 0, 0, 0}, {0, s / 2, 0, 0}, {0, 0, 1, 0}, {p.x, p.y + s / 2, 0, 1});
        tm[3]= float4x4({s / 2, 0, 0, 0}, {0, s, 0, 0}, {0, 0, 1, 0}, {p.x - s / 2, p.y, 0, 1});
    }

    void GetTextureSpaceMatrix(inout float4x4[4] tm)
    {
    tm[0] = float4x4({1.,0,0,0},{0,0.5,0,0.5},{0,0,1,0},{0.0,0.0,0,1});
    tm[1] = float4x4({0.5,0,0,0},{0,0.5,0,0},{0,0,1,0},{0,0,0,1});
    tm[2] = float4x4({0.5,0,0,0},{0,0.5,0,0},{0,0,1,0},{0,0,0,1});
    tm[3] = float4x4({0.5,0,0,0},{0,0.5,0,0},{0,0,1,0},{0,0,0,1});
    }

float GetClipDist(in float3 lightPos,float3 vertexPosition ,uint faceIndex, in uint planeIndex)
{
    float3 normal = -planeNormals[faceIndex*3 + planeIndex]; // clipping plane normal
    return (dot(vertexPosition, normal) + dot(-normal, lightPos));
}

struct Data{
ConstantBuffer<OmnidirectionalShadowmapData> data;
};

ParameterBlock<Data> LightData;

[shader("vertex")]
VSOutput vertexMain(VertexInput input)
{
    VSOutput output = {0};
    output.position = mul(UBO.model, float4(input.Position,1.0f));

    return output;
};

[shader("geometry")]
[maxvertexcount(12)]
void geometryMain(triangle VSOutput input[3], inout TriangleStream<GSOutput> triStream)
{
    uint lightIndex  = 0;
    //float3 lightPosition = LIGHTING.lights.pointLight[lightIndex].positionAndRange.xyz;
float3 lightPosition = float3(0,0,0);
 // back-face culling
    float3 normal = -cross(input[2].position.xyz-input[0].position.xyz, input[0].position.xyz - input[1].position.xyz);
    float3 viewDir = lightPosition-input[0].position.xyz;

   //TODO: INPLEMENT BACKFACE CULLING
    float4x4 tm[4];
    GetTextureSpaceMatrix(tm);

    float4x4 viewProjMatrices[4];


//View Proj Green Face [0]
    float3 forward = faceVectors[0];
    float3 up = cross(forward,{-1,0,0});
    float3x3 view3x3 = lookAt(forward,up);
    float4x4 view = {
    float4(view3x3[0], 0),
    float4(view3x3[1], 0),
    float4(view3x3[2], 0),
    float4(0, 0, 0, 1)
         };

    float fov_x = 143.98570868;
    float fov_y = 125.26438968;
    float4x4 proj =PerspectiveMatrixFOV(fov_x,fov_y,1,100);

    viewProjMatrices[0] = mul(proj,view);

    //View Proj Yellow Face [1]
          forward = faceVectors[1];
          up = cross(forward,{1,0,0});
          view3x3 = lookAt(forward,up);

          float4x4 view2 = {
            float4(view3x3[0], 0),
            float4(view3x3[1], 0),
            float4(view3x3[2], 0),
            float4(0, 0, 0, 1)
              };

          float4x4 proj2 =PerspectiveMatrixFOV(fov_y,fov_x,1,100);

          viewProjMatrices[1] = mul(proj2,view2);


//View Proj Blue Face [2]
          forward = faceVectors[2];
        up = cross(forward,{0.288674,0.408248,-0.866026});
  //      up = cross(forward, {-0.577349,-0.816497,0.0});

          view3x3 = lookAt(forward,up);

          float4x4 view3 = {
            float4(view3x3[0], 0),
            float4(view3x3[1], 0),
            float4(view3x3[2], 0),
            float4(0, 0, 0, 1)
              };

          float4x4 proj3 =PerspectiveMatrixFOV(fov_y,fov_x,1,100);

          viewProjMatrices[2] = mul(proj3,view3);

//View Proj Red Face [3]
          forward = faceVectors[3];
          up = cross(forward,{0,0,-1});
          view3x3 = lookAt(forward,{-0.577349,-0.816497,0.0});

          float4x4 view4 = {
            float4(view3x3[0], 0),
            float4(view3x3[1], 0),
            float4(view3x3[2], 0),
            float4(0, 0, 0, 1)
              };

          float4x4 proj4 =PerspectiveMatrixFOV(fov_y,fov_x,1,100);

          viewProjMatrices[3] = mul(proj4,view4);


float4x4 TextMatrices[4];
float4x4 textM_green = {
float4(0.25,0,0,-0.5),
float4(0,0.25,0,-.5),
float4(0,0,1,0),
float4(0,0,0,1)};

float4x4 textM_yellow = {
float4(1.,0,0,0),
float4(0,1.,0,-.5),
float4(0,0,1,0),
float4(0,0,0,1)
};

float4x4 textM_blue = {
float4(.5,0,0,-0.),
float4(0,.5,0,0),
float4(0,0,1,0),
float4(0,0,0,1)
};

float4x4 textM_red = {
float4(1.,0,0,0.),
float4(0,1.,0,0.),
float4(0,0,1,0),
float4(0,0,0,1)
};

TextMatrices[0] = textM_green;
TextMatrices[1] = textM_yellow;
TextMatrices[2] = textM_blue;
TextMatrices[3] = textM_red;

 for(uint faceIndex=2; faceIndex<3; faceIndex++)
 {
    //const float4x4 shadowViewProjTexMatrix = LightData.data.shadowMatrix[lightIndex*4 + faceIndex];
    float4x4 shadowViewProjMatrix = viewProjMatrices[faceIndex];
    float4x4 textM = TextMatrices[faceIndex];
    GSOutput output;

     for(uint vertexIndex=0; vertexIndex<3; vertexIndex++)
         {
            float3 vertexPos = input[vertexIndex].position.xyz;
            output.pos = mul(textM,mul(shadowViewProjMatrix,float4(input[vertexIndex].position.xyz,1)));

            float clipOffset = 0.;
            output.clipDistance[0] = GetClipDist(lightPosition,vertexPos, faceIndex,0) + clipOffset;
            output.clipDistance[1] = GetClipDist(lightPosition,vertexPos, faceIndex,1) + clipOffset;
            output.clipDistance[2] = GetClipDist(lightPosition,vertexPos, faceIndex,2) + clipOffset;

            triStream.Append(output);
         }
     triStream.RestartStrip();
 }
}

[shader("fragment")]
float4 fragmentMain(float4 input)
    : SV_Target
{
    return 1.0;
}

import PuduCoreModule;
import PuduGraphicsModule;



float3x3 lookAt(float3 forward, float3 up)
{
    float3 right = normalize(cross(forward, up));
    float3 newUp = normalize(cross(right, forward));
    return float3x3(right, newUp, forward);
}

struct VertexInput
{
 	[POSITION] float3 Position;
};

struct GSOutput
{
    float4 pos   : SV_POSITION;
    float clipDistance[3]: SV_ClipDistance;
    //float clipDistance[3];
};

struct VSOutput{
    float4 position: SV_POSITION;
};

static const float3 faceNormals[] = {
float3(-5.602816145255929e-06, -0.5773529410362244, 0.8164947032928467),
float3(-5.5730142776155844e-06, -0.5773529410362244, -0.8164947032928467),
float3(-0.8165003061294556, 0.577345073223114, 0.0),
float3(0.8164965510368347, 0.5773502588272095, 0.0),
};

static const float3 faceUps[] = {
float3(0.0, -0.8164947032928467, -0.5773529410362244),
float3(0.0, -0.8164947032928467, 0.5773529410362244),
float3(0.577345073223114, 0.8165003061294556, -0.0),
float3(0.5773503184318542, -0.8164966702461243, 0.0),
};

static const float3 clipPlaneNormals[] = {
float3(0.0, 0.0, 1.0),
float3(0.5, -0.7071068286895752, 0.5000000596046448),
float3(-0.5000041127204895, -0.7071067690849304, 0.49999597668647766),
float3(0.0, -0.0, -1.0),
float3(-0.5000041127204895, -0.7071067690849304, -0.49999597668647766),
float3(0.5, -0.7071068286895752, -0.5000000596046448),
float3(-0.5, 0.7071068286895752, 0.5000000596046448),
float3(-1.0, -5.729740223614499e-06, 0.0),
float3(-0.5, 0.7071068286895752, -0.5000000596046448),
float3(0.5000041127204895, 0.7071067690849304, -0.49999597668647766),
float3(1.0, 5.729740223614499e-06, 0.0),
float3(0.5000041127204895, 0.7071067690849304, 0.49999597668647766),
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
    float3 normal = normalize( clipPlaneNormals[faceIndex*3 + planeIndex]); // clipping plane normal
    return dot(normal, vertexPosition - lightPos); //Positive if front of plane, negative if behind
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


//pitch-x , roll-z , yaw-y

public float3x3 RotationMatrix3(float3 rotation)
{
    float roll = rotation.z;
    float pitch = rotation.x;
    float yaw = rotation.y;

    float su = sin(pitch);
    float cu = cos(pitch);
    float sv = sin(yaw);
    float cv = cos(yaw);
    float sw = sin(roll);
    float cw = cos(roll);

    float3x3 rotate_x = {
        {1, 0, 0},
        {0, cu, -su},
        {0, su, cu},

    };

    float3x3 rotate_y = {
        {cv, 0, sv},
        {0, 1, 0},
        {-sv, 0, cv}
    };

    float3x3 rotate_z = {
        {cw, -sw, 0},
        {sw, cw, 0},
        {0, 0, 1}
    };

    return mul(rotate_x, mul(rotate_y, rotate_z));
    }

public float4x4 RotationMatrix(float3 rotation)
{
    float roll = rotation.z;
    float pitch = rotation.x;
    float yaw = rotation.y;

    float su = sin(pitch);
    float cu = cos(pitch);
    float sv = sin(yaw);
    float cv = cos(yaw);
    float sw = sin(roll);
    float cw = cos(roll);

    float4x4 rotate_x = {
        {1, 0, 0, 0},
        {0, cu, -su, 0},
        {0, su, cu, 0},
        {0, 0, 0, 1}
    };

    float4x4 rotate_y = {
        {cv, 0, sv, 0},
        {0, 1, 0, 0},
        {-sv, 0, cv, 0},
        {0, 0, 0, 1}
    };

    float4x4 rotate_z = {
        {cw, -sw, 0, 0},
        {sw, cw, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };

    return mul(rotate_x, mul(rotate_y, rotate_z));
    }

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
    float3 forward = faceNormals[0];
    float3 up = faceUps[0];
    float3x3 view3x3 = lookAt(forward,up);

    float4x4 view = RotationMatrix(float3(radians(-27.36780516),radians(0),0));
    float fov_x = 143.98570868;
    float fov_y = 125.26438968;

    float4x4 proj = PerspectiveMatrixFOV(fov_y,fov_x,1,100);

    viewProjMatrices[0] = mul(proj,view);

    //View Proj Yellow Face [1]
          forward = faceNormals[1];
          up = faceUps[1];

          float4x4 view2 = RotationMatrix(float3(radians(-27.36780516),radians(180),radians(0)));

          float4x4 proj2 = PerspectiveMatrixFOV(fov_y,fov_x,1,100);

          viewProjMatrices[1] = mul(proj2,view2);


//View Proj Blue Face [2]
        forward = faceNormals[2];
        up = faceUps[2];
        view3x3 = RotationMatrix3(float3(radians(27.36780516),radians(90),radians(0)));

        float4x4 view3 = RotationMatrix(float3(radians(27.36780516),radians(90),radians(0)));
        float4x4 proj3 =PerspectiveMatrixFOV(fov_y,fov_x,1,100);

          viewProjMatrices[2] = mul(proj3,view3);

//View Proj Red Face [3]
            //VERYFY HANDEDNESS SLANG
          forward = faceNormals[3];
          up = faceUps[3];
          view3x3 = lookAt(forward,-up);

          float4x4 view4 = RotationMatrix(float3(radians(27.36780516),radians(-90),radians(0)));

          float4x4 proj4 =PerspectiveMatrixFOV(fov_y,fov_x,1,100);

          viewProjMatrices[3] = mul(proj4,view4);


float4x4 TextMatrices[4];
float p = 0.5;
float4x4 textM_green = {
float4(p,0,0,-p),
float4(0,p,0,-p),
float4(0,0,1,0),
float4(0,0,0,1)};

float4x4 textM_yellow = {
float4(p,0,0,p),
float4(0,p,0,-p),
float4(0,0,1,0),
float4(0,0,0,1)
};

float4x4 textM_blue = {
float4(p,0,0,-p),
float4(0,p,0,p),
float4(0,0,1,0),
float4(0,0,0,1)
};

float4x4 textM_red = {
float4(p,0,0,0.5),
float4(0,p,0,0.5),
float4(0,0,1,0),
float4(0,0,0,1)
};

TextMatrices[0] = textM_green;
TextMatrices[1] = textM_yellow;
TextMatrices[2] = textM_blue;
TextMatrices[3] = textM_red;

 for(uint faceIndex=0; faceIndex<4; faceIndex++)
 {
    //const float4x4 shadowViewProjTexMatrix = LightData.data.shadowMatrix[lightIndex*4 + faceIndex];
    float4x4 shadowViewProjMatrix = viewProjMatrices[faceIndex];
    float4x4 textM = TextMatrices[faceIndex];
    GSOutput output;

     for(uint vertexIndex=0; vertexIndex<3; vertexIndex++)
         {
            float3 vertexPos = input[vertexIndex].position.xyz;
            output.pos =mul(textM, mul(shadowViewProjMatrix,float4(input[vertexIndex].position.xyz,1)));

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

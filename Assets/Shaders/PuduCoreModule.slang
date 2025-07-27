module PuduCore;
public enum BlendingMode
{
        Opaque,
		AlphaBlend,
		AdditiveBlend,
		SubtractiveBlend,
		MultiplyBlend,
		CustomBlend
};

public enum BlendOp
{
    ADD = 0,
    SUBTRACT = 1,
    REVERSE_SUBTRACT = 2,
    MIN = 3,
    MAX = 4,
    ZERO_EXT = 1000148000,
    SRC_EXT = 1000148001,
    DST_EXT = 1000148002,
    SRC_OVER_EXT = 1000148003,
    DST_OVER_EXT = 1000148004,
    SRC_IN_EXT = 1000148005,
    DST_IN_EXT = 1000148006,
    SRC_OUT_EXT = 1000148007,
    DST_OUT_EXT = 1000148008,
    SRC_ATOP_EXT = 1000148009,
    DST_ATOP_EXT = 1000148010,
    XOR_EXT = 1000148011,
    MULTIPLY_EXT = 1000148012,
    SCREEN_EXT = 1000148013,
    OVERLAY_EXT = 1000148014,
    DARKEN_EXT = 1000148015,
    LIGHTEN_EXT = 1000148016,
    COLORDODGE_EXT = 1000148017,
    COLORBURN_EXT = 1000148018,
    HARDLIGHT_EXT = 1000148019,
    SOFTLIGHT_EXT = 1000148020,
    DIFFERENCE_EXT = 1000148021,
    EXCLUSION_EXT = 1000148022,
    INVERT_EXT = 1000148023,
    INVERT_RGB_EXT = 1000148024,
    LINEARDODGE_EXT = 1000148025,
    LINEARBURN_EXT = 1000148026,
    VIVIDLIGHT_EXT = 1000148027,
    LINEARLIGHT_EXT = 1000148028,
    PINLIGHT_EXT = 1000148029,
    HARDMIX_EXT = 1000148030,
    HSL_HUE_EXT = 1000148031,
    HSL_SATURATION_EXT = 1000148032,
    HSL_COLOR_EXT = 1000148033,
    HSL_LUMINOSITY_EXT = 1000148034,
    PLUS_EXT = 1000148035,
    PLUS_CLAMPED_EXT = 1000148036,
    PLUS_CLAMPED_ALPHA_EXT = 1000148037,
    PLUS_DARKER_EXT = 1000148038,
    MINUS_EXT = 1000148039,
    MINUS_CLAMPED_EXT = 1000148040,
    CONTRAST_EXT = 1000148041,
    INVERT_OVG_EXT = 1000148042,
    RED_EXT = 1000148043,
    GREEN_EXT = 1000148044,
    BLUE_EXT = 1000148045,
}

public enum BlendFactor
{
    ZERO = 0,
    ONE = 1,
    SRC_COLOR = 2,
    ONE_MINUS_SRC_COLOR = 3,
    DST_COLOR = 4,
    ONE_MINUS_DST_COLOR = 5,
    SRC_ALPHA = 6,
    ONE_MINUS_SRC_ALPHA = 7,
    DST_ALPHA = 8,
    ONE_MINUS_DST_ALPHA = 9,
    CONSTANT_COLOR = 10,
    ONE_MINUS_CONSTANT_COLOR = 11,
    CONSTANT_ALPHA = 12,
    ONE_MINUS_CONSTANT_ALPHA = 13,
    SRC_ALPHA_SATURATE = 14,
    SRC1_COLOR = 15,
    ONE_MINUS_SRC1_COLOR = 16,
    SRC1_ALPHA = 17,
    ONE_MINUS_SRC1_ALPHA = 18,
}

public enum CullingMode
{
Back,
Front,
None
}

public enum DefaultTexture
{
White,
Black,
Normal,
Pudu
}

[__AttributeUsage(_AttributeTargets.Var)]
struct BindlessAttribute
{
};

[__AttributeUsage(_AttributeTargets.Var)]
struct ScopeAttribute
{
    string value;
};

[__AttributeUsage(_AttributeTargets.Var)]
struct DefaultTextureAttribute
{
    DefaultTexture value;
};

[__AttributeUsage(_AttributeTargets.Var)]
struct NormalMap
{};


[__AttributeUsage(_AttributeTargets.Function)]
struct SrcColorFactorAttribute{
BlendFactor factor;
}

[__AttributeUsage(_AttributeTargets.Function)]
struct DstColorFactorAttribute{
BlendFactor factor;
}

[__AttributeUsage(_AttributeTargets.Function)]
struct SrcAlphaFactorAttribute{
BlendFactor factor;
}

[__AttributeUsage(_AttributeTargets.Function)]
struct DstAlphaFactorAttribute{
BlendFactor factor;
}

[__AttributeUsage(_AttributeTargets.Function)]
struct ColorBlendOpAttribute{
BlendOp op;
}

[__AttributeUsage(_AttributeTargets.Function)]
struct AlphaBlendOpAttribute{
BlendOp op;
}

[__AttributeUsage(_AttributeTargets.Function)]
struct BlendingAttribute{
  BlendingMode blendingMode;

   __init(BlendingMode blendingMode)
      {
          this.blendingMode = blendingMode;
      };
};


[__AttributeUsage(_AttributeTargets.Function)]
struct CullingAttribute{
CullingMode cullMode;
};

public static const float PI = 3.14159265358979f;
public static constexpr float PI_HALF = PI/2;
public static constexpr float TAU = PI*2;
public static constexpr float PI_INVERSE = 1./PI;
public static constexpr float PI_INVERSE_HALF = PI_INVERSE*0.5f;


public float LinearizeDepth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

public float3 DecodeNormal(float2 n)
{
    float2 m = n*2. - 1.;
    var z = sqrt(1. - m.x*m.x - m.y*m.y);

    return normalize(float3(m,z));
}

public float3 TangentToWorld(float3 v, float3 t, float3 bt, float3 n)
{
    return normalize( v.x*t + v.y*bt + v.z*n);
}

//<summary> TBN generates a tangent bitangent normal coordinate frame from the normal (the normal must be normalized)</summary>
public float3x3 GenerateTBN(float3 Normal) {
  float3 bitangent = float3(0.0, 1.0, 0.0);

  float NdotUp = dot(Normal, float3(0.0, 1.0, 0.0));
  float epsilon = 0.0000001;
  if (1.0 - abs(NdotUp) <= epsilon) {
    // Sampling +Y or -Y, so we need a more robust bitangent.
    bitangent = (NdotUp > 0.0) ? float3(0.0, 0.0, 1.0) : float3(0.0, 0.0, -1.0);
  }

  float3 tangent = normalize(cross(bitangent, Normal));
  bitangent = cross(Normal, tangent);

    //PUDU RENDERER IS IN COLUMN MAJOR SO WE NEED TO TRANSPOSE IT
  return transpose(float3x3(tangent, bitangent, Normal));
}

public float3 WorldToTangent(float3 v, float3 t, float3 bt, float3 n)
{
    return mul(float3x3(
        t,
        bt,
        n
    ), v);
}

public float2 ParallaxUV(float h, float2 uv, float3 viewDir, float scale, float3 t, float3 bt,float3 n)
{
    float3 tangentView = normalize(WorldToTangent(viewDir,t,bt, n));

    return uv + h*scale*tangentView.xy;
}

//Returns line intersection, it assume that lines intersect
public float3 Line3D_Intersect(float3 pA0, float3 pA1, float3 pB0, float3 pB1)
{
    //Parametrization of linear equations system generated by making line A equal to line B
    float3 A = pA0 - pB0;
    float3 u = pA1 - pA0;
    float3 v = pB1 - pB0;

    float t = ((v.x * A.y) - (v.y * A.x))/((v.y * u.x) - (v.x * u.y));

    return pA0 + (pA1 - pA0) * t;
}

public float2 ParallaxOcclusionMapping(Sampler2D heightMap, float2 uv, float3 viewDirTS, float scale) {
    const float minLayers = 16;
    const float maxLayers = 32;
    float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), viewDirTS)));

    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    float2 P = viewDirTS.xy / viewDirTS.z * scale;
    float2 deltaTexCoords = P / numLayers;

    // get initial values
    float2 currentUv = uv;
    float currentDepthMapValue = 1.0 - heightMap.Sample(currentUv).r;

    while (currentLayerDepth < currentDepthMapValue) {
        // shift texture coordinates along direction of P
        currentUv -= deltaTexCoords;
        // get depth of current layer
        currentDepthMapValue = 1.0 - heightMap.Sample(currentUv).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    return currentUv;
}

public struct POMS_OUT
{
    public float2 uv;
    public float shadow;
    }
public POMS_OUT ParallaxOcclusionMapping_Shadow(Sampler2D heightMap, float2 uv, float3 viewDirTS,float3 lightDirTS, float scale) {
    const float minLayers = 128;
    const float maxLayers = 512;
    float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), viewDirTS)));

    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 1.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    float2 P = viewDirTS.xy / viewDirTS.z * scale;
    float2 deltaTexCoords = -P / numLayers;

    // get initial values
    float2 currentUv = uv;
    float currentDepthMapValue = heightMap.Sample(currentUv).r;

    float3 previousPos;
    float previousDepth;

    while (currentLayerDepth > currentDepthMapValue) {
        previousPos = float3(currentUv, currentLayerDepth);
        previousDepth = currentDepthMapValue;

        currentUv += deltaTexCoords;
        currentDepthMapValue = heightMap.Sample(currentUv).x;
        currentLayerDepth -= layerDepth;
    }

    float3 currentPos = float3(currentUv,currentLayerDepth);

    float3 pA0 = previousPos;
    float3 pA1 = currentPos;

    float3 pB0 = float3(previousPos.xy,previousDepth);
    float3 pB1 = float3(currentPos.xy,currentDepthMapValue);

    float3 intersectionPoint = Line3D_Intersect(pA0,pA1,pB0,pB1);

    currentUv = intersectionPoint.xy;

    //UV Found. Look for shadow
    numLayers = lerp(maxLayers,minLayers,abs(dot(float3(0,0,1),lightDirTS)));
    P = lightDirTS.xy/lightDirTS.z * scale;
    layerDepth = 1./numLayers;
    deltaTexCoords = -P/numLayers;

    float shadowLayerDepth = currentDepthMapValue;
    float2 shadowUv = currentUv;
    float shadow = 1.;
    while(shadowLayerDepth<1)
    {
        shadowUv +=deltaTexCoords;
        currentDepthMapValue = heightMap.Sample(shadowUv).r;

        shadowLayerDepth+=layerDepth;

        if(shadowLayerDepth<currentDepthMapValue)
        {
            shadow = 0.0;
            break;
        }
    }

    return {currentUv,shadow};
}


// Vulkan Vertical Cross Cubemap Layout (Right-Handed, Y-Up):
// Layout for a vertical cross cubemap in a right-handed Y-Up coordinate system looks like this:
//       +----+
//       | +Y |
//  +----+----+----+
//  | -X | -Z | +X |
//  +----+----+----+
//       | -Y |
//       | +Z |
//       +----+
// Coordinates are remapped by "looking" at the given face
public float3 CubeCoordToDir(float u, float v, uint faceID)
{
     float3 d = 0.0001;
     if (faceID == 0) d = float3(   1.,-v,-u);
     if (faceID == 1) d =  float3(-1.,    -v,u);
     if (faceID == 2) d =  float3(    u, 1.,v);
     if (faceID == 3) d =  float3(u,-1.f,-v);
     if (faceID == 4) d =  float3(u,-v,1.f);
     if (faceID == 5) d = float3(-u, -v,-1.f);
     return normalize(d);
}

public float2 DirToLongitudeLatitude(float3 dir)
{
    float theta = atan2(dir.x,dir.z);
    float phi = acos(dir.y);

    return float2(theta*PI_INVERSE_HALF + 0.5, phi * PI_INVERSE);
}


//<summary>Converts spherical coordinates (phi, theta) to Cartesian coordinates (x,y,z). Phi represents azimuthal angle and theta represents polar (zenit) angle</summary>
public float3 SphericalToCartesian(float2 phiTheta)
{
    float x = cos(phiTheta.x);
    float z = sin(phiTheta.x);
    float y = cos(phiTheta.y);

    return float3(x,y,z);
}

//<summary>Converts spherical coordinates (phi, theta) to Cartesian coordinates (x,y,z). Phi represents azimuthal angle and theta represents polar (zenit) angle</summary>
public float3 SphericalToCartesian(float phi, float theta)
{
    return SphericalToCartesian({phi,theta});
}

public float3 ToneMap_Reinhard(float3 color, float exposure)
{
    color *= exposure;
    return color / (color + float3(1.0)); // Basic Reinhard tonemapping
}

public float3 ToneMap_Reinhard(float4 color, float exposure)
{
    return ToneMap_Reinhard(color.xyz,exposure);
}

public float3 GammaCorrect(float3 color, float gamma)
{
    return pow(color,1./gamma);
}

public inline float Max3(float3 v){
    return max(max(v.x,v.y),v.z);
}
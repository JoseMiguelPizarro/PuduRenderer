import  PuduCoreModule;
#include "Lib/Lighting.hlsl"

module PuduGraphicsModule;

[__AttributeUsage(_AttributeTargets.Var)]
struct BindlessAttribute
{
};

[__AttributeUsage(_AttributeTargets.Var)]
struct ScopeAttribute
{
    string value;
};

public struct GlobalsConstants
{
    public float2 screenSize;
    public float time;
    public float nearPlane;
    public float farPlane;
    public float3 cameraPosWS;
    public float4x4 view;
    public float4x4 proj;
}

public struct Globals
{
    public Sampler2DShadow shadowMap;
    public Sampler2D normalBuffer;
    public Sampler2D depthBuffer;
    public Sampler2D colorBuffer;
    public SamplerCube skybox;
    public SamplerCube IBL_Specular; //Convoluted image based lighting env map
    public SamplerCube IBL_Diffuse;
    public uint IBL_Levels;
    public Sampler2D BRDF_LUT;
    public SamplerState LinearSampler;
    public ConstantBuffer<LightBuffer> lightingBuffer;
    public ConstantBuffer<GlobalsConstants> constants;
    public float gamma = 2.2;
}

public enum DebugMode
{
   None,Albedo,Diffuse,Normal, Metallic, Roughness, Emissive, LightSpecular, LightDiffuse, ShadowAttenuation, DirectLight, Irradiance, BRDFLUT,
   VertexNormal,VertexTangent,VertexColor,UV0
};

public struct DEBUG
{
  public DebugMode debugMode;
  public bool toneMapping = false;
  public float roughnessScale = 1;
}

// public struct GlobalsBindless
// {
//     public Sampler2D textures[];
// }

[Scope("Global")]
public ParameterBlock<Globals> GLOBALS;

[Scope("Global")]
public ParameterBlock<DEBUG> DEBUG_SETTINGS;

// [Bindless, Scope("Global")]
// public ParameterBlock<GlobalsBindless> GLOBALS_BINDLESS;

[vk::push_constant]
public UniformBufferObject UBO;

#define PERSHADERBIND 2
#define BIND(id) [[vk::binding(id, PERSHADERBIND)]]
#define CONSTANTS(type)   \
    [[vk::push_constant]] \
    ConstantBuffer<type>

// public Sampler2D GetGlobalTexture(uint id)
// {
//     return GLOBALS_BINDLESS.textures[NonUniformResourceIndex(id)];
// }

//This should only be called during the fragment shader. in fragment shader SV_Position is effectively the screen pixel coordinates
public float2 GetPixelScreenPosition(float4 PosCS)
{
    return float2(PosCS.x/GLOBALS.constants.screenSize.x,PosCS.y/GLOBALS.constants.screenSize.y);
};

public float LinearizeDepth(float d)
{
    return LinearizeDepth(d, GLOBALS.constants.nearPlane, GLOBALS.constants.farPlane);
};

public float4x4 GetViewMatrix(){
    return GLOBALS.constants.view;
}


/// <summary>
/// Computes the normalized view direction vector in world space.
/// </summary>
/// <param name="worldPos">The position in world space from which the view direction is computed.</param>
/// <returns>
/// A float3 vector representing the direction from the given world position to the camera position,
/// </returns>
public float3 GetViewDirection(float3 worldPos)
{
    return normalize(GLOBALS.constants.cameraPosWS  - worldPos);
}

public float4x4 GetProjectionMatrix()
{
    return GLOBALS.constants.proj;
}


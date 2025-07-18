import PuduCoreModule;
module PuduDebugModule;

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
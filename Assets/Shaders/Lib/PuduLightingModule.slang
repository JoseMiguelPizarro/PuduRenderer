module PuduLightingModule;

import PuduCoreModule;
import PuduRandomModule;
import PuduGraphicsModule;


#define BlinnPhongSpecularAlpha 50.f
static const float DielectricSpecular = 0.04f;

public interface IDiffuseAmbientSampler {
    public float3 Sample(float3 N);
}

public struct IBLAmbientSampler: IDiffuseAmbientSampler{

    public float3 Sample(float3 N){
        return float3(0.);}
    }

public struct SurfaceInfo {
    public float3 Albedo;
    public float3 Normal;
    public float3 ViewDir;
    public float3 LightDir;
    public float3 LightColor;
    public float3 SpecularColor = float3(1, 1, 1);
    public float Glossiness;
    public float Metallic;
    public float ShadowAttenuation;
};

float G_SchlicksmithGGX(
  float NdL, float NdV, float Roughness)
{
  float K  = (Roughness * Roughness) / 2.0;
  float GL = NdL / (NdL * (1.0 - K) + K);
  float GV = NdV / (NdV * (1.0 - K) + K);
  return GL * GV;
};

float D_GGX(float NdotH, float roughness) {
  float a = NdotH * roughness;
  float k = roughness / (1.0 - NdotH * NdotH + a * a);
  return k * k * (1.0 / PI);
}

public struct MicrofacetDistributionSample {
  public float pdf; //Probability Distribution Function
  public float cosTheta;
  public float sinTheta;
  public float phi;
};

// GGX microfacet distribution
// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.html
// This implementation is based on https://bruop.github.io/ibl/,
//  https://www.tobias-franke.eu/log/2014/03/30/notes_on_importance_sampling.html
// and https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html
public MicrofacetDistributionSample GGX(float2 Xi, float Roughness) {
  MicrofacetDistributionSample ggx;

  // evaluate sampling equations
  float alpha = Roughness * Roughness;
  ggx.cosTheta = saturate(sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0) * Xi.y)));
  ggx.sinTheta = sqrt(1.0 - ggx.cosTheta * ggx.cosTheta);
  ggx.phi = 2.0 * PI * Xi.x;

  // evaluate GGX pdf (for half vector)
  ggx.pdf = D_GGX(ggx.cosTheta, alpha);

  // Apply the Jacobian to obtain a pdf that is parameterized by l see https://bruop.github.io/ibl/
  // Typically you'd have the following:
  // float pdf = D_GGX(NoH, roughness) * NoH / (4.0 * VoH);
  // but since V = N => VoH == NoH
  ggx.pdf /= 4.0;

  return ggx;
}


float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N) {
     float alpha = Roughness * Roughness;
     float phi = 2 * PI * Xi.x + random(N.xz) * 0.1;
     float cosTheta  = sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0) * Xi.y));
     float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
     float3 H = float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
      float3 up = abs(N.z) < 0.999 ?
         float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
       float3 tangentX = normalize(cross(up, N));
       float3 tangentY = normalize(cross(N, tangentX));
       return normalize(tangentX * H.x +
                        tangentY * H.y +
                          N * H.z);
}

const static uint K_Num_Samples = 1024;
///<summary>Computes BRDF.x: F0 scale. y:F0 bias</summary>
public float3 BRDF(float Ndv, float Roughness) {
    const float3 N = float3(0.0, 0.0, 1.0);
    float3 V   = float3(sqrt(1.0 - Ndv * Ndv), 0.0, Ndv);
    float3 Lut = float3(0.0);

    for (uint I = 0u; I < K_Num_Samples; I++) {
      float2 Xi = HammersleyHemisphere(I, K_Num_Samples);
      float3 H = ImportanceSampleGGX(Xi, Roughness, N);
      float3 L = 2.0 * dot(V, H) * H - V;
      float NdL = max(dot(N, L), 0.0);
      float NdV = max(dot(N, V), 0.0);
      float VdH = max(dot(V, H), 0.0);
      float NdH = max(dot(H, N), 0.0);
      if (NdL > 0.0) {
        float G = G_SchlicksmithGGX(NdL, NdV, Roughness);
        float G_Vis = (G * VdH) / (NdH * NdV);
        float Fc = pow(1.0 - VdH, 5.0);
        Lut.rg += float2((1.0 - Fc) * G_Vis, Fc * G_Vis);
      }
    }

    return Lut / float(K_Num_Samples);
}

float3 SampleAmbientSpecular(SurfaceInfo Surface) {
    float3 R = reflect(Surface.ViewDir, Surface.Normal);
    return 0;
}

float3 SampleAmbientDiffuse(SurfaceInfo Surface) {
    return 0;
}

float3 BlinnPhongSpecularReflection(SurfaceInfo Surface) {
    float3 L = Surface.LightDir;
    float3 N = Surface.Normal;
    float3 V = Surface.ViewDir;

    float3 H = normalize(L + V);

    float3 S = pow(Surface.SpecularColor * Surface.LightColor * saturate(dot(N, H)), BlinnPhongSpecularAlpha) * Surface.Glossiness;
    S += SampleAmbientSpecular(Surface);

    return S;
}

float3 PhongSpecularReflection(SurfaceInfo Surface) {
    float3 L = Surface.LightDir;
    float3 N = Surface.Normal;
    float3 V = Surface.ViewDir;
    float3 R = reflect(Surface.LightDir, Surface.Normal);

    float3 S = pow(Surface.SpecularColor * Surface.LightColor * saturate(dot(V, R)), 5 * (Surface.Glossiness));

    S = S + SampleAmbientSpecular(Surface);
    return S;
}

float3 LambertDiffuseReflection(SurfaceInfo Surface) {
    float3 DirectColor = Surface.LightColor * saturate(dot(Surface.Normal, Surface.LightDir));

    return DirectColor * Surface.Albedo + SampleAmbientDiffuse(Surface); // Missing diffuse ambient color here!
}

float3 LambertBRDF(SurfaceInfo Surface) {
    float3 DiffuseReflection = LambertDiffuseReflection(Surface);
    float3 SpecularReflection = BlinnPhongSpecularReflection(Surface);

    return GLOBALS.IBL.SampleLevel(Surface.Normal,0).xyz;
}

public float3 StandardSurface(SurfaceInfo Surface) {
    float NdV = dot(Surface.Normal, Surface.ViewDir);
    float3 diffuseColor = Surface.Albedo.rgb * (1.0 - DielectricSpecular) * (1.0 - Surface.Metallic);
    float3 F0 = lerp(DielectricSpecular.xxx,Surface.Albedo.rgb,Surface.Metallic);
    float2 f_ab = GLOBALS.BRDF_LUT.Sample({NdV,1. - Surface.Glossiness}).xy;

    return LambertBRDF(Surface);
}
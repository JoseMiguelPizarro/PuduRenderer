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
    public float3 SpecularColor = float3(1, 1, 1);
    public float3 F0 = float3(.2,.2,.2);
    public float3 F90 = float3(1,1,1);
    public float3 Emissive;
    public float Roughness;
    public float Metallic;
    public float ShadowAttenuation;
};

struct PBRInfo
{
    float3 ViewDir;
    float3 DiffuseColor;
    float3 SpecularColor;
    float3 VrN;
    float LdH;
    float NdV;
    float NdL;
    float NdH;
    float VdH;
    float alphaRoughness;
}

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
public MicrofacetDistributionSample DGGX_ImportanceSample(float2 Xi, float Roughness) {
  MicrofacetDistributionSample ggx;

  // evaluate sampling equations
  float alpha = Roughness * Roughness;
  ggx.cosTheta = saturate(sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0) * Xi.y))); //cosTheta comes from the inverse GGX_D CDF. Taking xi.y (uniform random value) as input. Derivation can be found on Filament docs
  ggx.sinTheta = sqrt(1.0 - ggx.cosTheta * ggx.cosTheta);
  ggx.phi = TAU * Xi.x;

  // evaluate GGX pdf (for half vector)
  ggx.pdf = D_GGX(ggx.cosTheta, alpha);

  // Apply the Jacobian to obtain a pdf that is parameterized by l see https://bruop.github.io/ibl/
  // Typically you'd have the following:
  // float pdf = D_GGX(NoH, roughness) * NoH / (4.0 * VoH);
  // but since V = N => VoH == NoH
  ggx.pdf /= 4.0;

  return ggx;
}

MicrofacetDistributionSample LambertianImportanceSample(float2 xi) {
  MicrofacetDistributionSample lambertian;

  // Cosine weighted hemisphere sampling
  // http://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations.html#Cosine-WeightedHemisphereSampling
  lambertian.cosTheta = sqrt(1.0 - xi.y);
  lambertian.sinTheta = sqrt(xi.y); // equivalent to `sqrt(1.0 - cosTheta*cosTheta)`;
  lambertian.phi = 2.0 * TAU * xi.x;

  lambertian.pdf = lambertian.cosTheta / PI; // evaluation for solid angle, therefore drop the sinTheta

  return lambertian;
}

public float4 GetLambertianImportanceSample(uint SampleIndex, float3 N,uint SampleCount)
{
        float3 irradiance = 0;
        float weight = 0;

         float3 up = abs(N.z) < 0.999 ?
         float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
         float3 tangentX = normalize(cross(up, N));
         float3 tangentY = normalize(cross(N, tangentX));

        float2 xi = HammersleyHemisphere(SampleIndex, SampleCount);

        MicrofacetDistributionSample dist = LambertianImportanceSample(xi);
        float cosPhi = cos(dist.phi);
        float sinPhi = sin(dist.phi);

        float3 directionTS = float3(dist.sinTheta * cosPhi,dist.sinTheta*sinPhi,dist.cosTheta);

        float3 H = normalize(tangentX * directionTS.x +
                                    tangentY * directionTS.y +
                                      N * directionTS.z);

        return float4(H,dist.pdf);
}

///<summary>Returns microfacet H vector using importance sampling with a DGGX PDF</summary>
public float4 GetDGGXImportanceSample(uint SampleIndex, float3 N, float Roughness, uint SampleCount)
{
    float2 xi = HammersleyHemisphere(SampleIndex,SampleCount);
    MicrofacetDistributionSample importanceSample = DGGX_ImportanceSample(xi,Roughness);

    float3 directionTS = normalize(
        float3(
        importanceSample.sinTheta * cos(importanceSample.phi),
        importanceSample.sinTheta * sin(importanceSample.phi),
        importanceSample.cosTheta));

     float3 up = abs(N.z) < 0.999 ?
     float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
     float3 tangentX = normalize(cross(up, N));
     float3 tangentY = normalize(cross(N, tangentX));
     float3 H = normalize(tangentX * directionTS.x +
                            tangentY * directionTS.y +
                              N * directionTS.z);

    return float4(H,importanceSample.pdf);
}

const static uint K_Num_Samples = 1024;
///<summary>Computes BRDF.x: F0 scale. y:F0 bias</summary>
public float3 BRDF(float Ndv, float Roughness) {
    const float3 N = float3(0.0, 0.0, 1.0);
    float3 V   = float3(sqrt(1.0 - Ndv * Ndv), 0.0, Ndv);
    float3 Lut = float3(0.0);

    for (uint I = 0u; I < K_Num_Samples; I++) {
      float3 H = GetDGGXImportanceSample(I,N,Roughness,K_Num_Samples).xyz;
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

float3 DiffuseLambert(PBRInfo PBR, SurfaceInfo Surface)
{
    return PBR.DiffuseColor/PI;
}

///<summary>From Disney http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdfhttp://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf. </summary>
float3 DiffuseBurley(PBRInfo PBR, SurfaceInfo Surface)
{
     float f90 = 2.0 * PBR.LdH * PBR.LdH *
        PBR.alphaRoughness - 0.5;
      return (PBR.DiffuseColor / PI) * (1.0 + f90 * pow((1.0 - PBR.NdL), 5.0)) * (1.0 + f90 * pow((1.0 - PBR.NdV), 5.0));
}

float3 FresnelSpecularReflection(PBRInfo PBR, SurfaceInfo Surface)
{
     return Surface.F0 +
        (Surface.F90 - Surface.F0) *
         pow(saturate(1.0 - PBR.VdH), 5.0);
}

float GeometricOcclusion(PBRInfo PBR) {
  float NdotL = PBR.NdL;
  float NdotV = PBR.NdV;
  float rSqr = PBR.alphaRoughness * PBR.alphaRoughness;
  float attenuationL = 2.0 * NdotL /
    (NdotL + sqrt(rSqr + (1.0 - rSqr) * (NdotL * NdotL)));
  float attenuationV = 2.0 * NdotV /
    (NdotV + sqrt(rSqr + (1.0 - rSqr) * (NdotV * NdotV)));
  return attenuationL * attenuationV;
}

float MicrofacetDistribution(PBRInfo PBR) {
  float roughnessSq = PBR.alphaRoughness * PBR.alphaRoughness;
  float f = (PBR.NdH * roughnessSq - PBR.NdH) *
    PBR.NdH + 1.0;
  return roughnessSq / (PI * f * f);
}


float3 SampleAmbientSpecular(SurfaceInfo Surface) {
    float3 R = reflect(Surface.ViewDir, Surface.Normal);
    return 0;
}

float3 SampleAmbientDiffuse(SurfaceInfo Surface) {
    return 0;
}

// float3 BlinnPhongSpecularReflection(PBRInfo PBR, SurfaceInfo Surface) {
//     float3 L = Surface.LightDir;
//     float3 N = Surface.Normal;
//     float3 V = Surface.ViewDir;
//
//     float3 H = normalize(L + V);
//
//     float3 S = pow(Surface.SpecularColor * Surface.LightColor * saturate(dot(N, H)), BlinnPhongSpecularAlpha) * (1. - Surface.Roughness);
//     S += SampleAmbientSpecular(Surface);
//
//     return S;
// }
//
// float3 PhongSpecularReflection(SurfaceInfo Surface) {
//     float3 L = Surface.LightDir;
//     float3 N = Surface.Normal;
//     float3 V = Surface.ViewDir;
//     float3 R = reflect(Surface.LightDir, Surface.Normal);
//
//     float3 S = pow(Surface.SpecularColor * Surface.LightColor * saturate(dot(V, R)), 5 * (1. - Surface.Roughness));
//
//     S = S + SampleAmbientSpecular(Surface);
//     return S;
// }
//
// float3 LambertDiffuseReflection(SurfaceInfo Surface) {
//     float3 DirectColor = Surface.LightColor * saturate(dot(Surface.Normal, Surface.LightDir));
//
//     return DirectColor * Surface.Albedo + SampleAmbientDiffuse(Surface); // Missing diffuse ambient color here!
// }

// float3 LambertBRDF(SurfaceInfo Surface) {
//     float3 DiffuseReflection = LambertDiffuseReflection(Surface);
//     float3 SpecularReflection = BlinnPhongSpecularReflection(Surface);
//
//     return GLOBALS.IBL_Specular.SampleLevel(Surface.Normal,0).xyz;
// }

float2 SampleBRDFLUT(float2 RoughnessNdV)
{
    return GLOBALS.BRDF_LUT.Sample(RoughnessNdV).rg;
}

float3 SampleIrradiance(float3 N)
{
    return GLOBALS.IBL_Diffuse.Sample(N).rgb;
}

float3 GetIBLRadianceDiffuse(PBRInfo PBR, SurfaceInfo Surface,float SpecularWeight)
{
    float2 brdfSamplePoint = saturate(float2(PBR.NdV,Surface.Roughness));

    float3 F0 = Surface.F0;
    float2 f_ab = SampleBRDFLUT(brdfSamplePoint);
    float3 irradiance = SampleIrradiance(Surface.Normal);
    float3 fr = max(1. - Surface.Roughness, Surface.F0) - F0;
    float3 k_s  = F0 + fr * pow(1. - PBR.NdV,5.);
    float3 FssEss = SpecularWeight*k_s *f_ab.x + f_ab.y;
    float Ems = (1. - (f_ab.x + f_ab.y));
    float3 F_avg = SpecularWeight * (F0 + (1.0 - F0) / 21.0);
    float3 FmsEms = Ems * FssEss * F_avg / max(1.0 - F_avg * Ems,0.0001);
    float3 k_D = PBR.DiffuseColor * (1.0 - FssEss + FmsEms);
    return (FmsEms + k_D) * irradiance;
}

float3 GetIBLRadianceContributionGGX(PBRInfo PBR, SurfaceInfo Surface , float SpecularWeight)
{
    float3 n = Surface.Normal;
    float3 v = Surface.ViewDir;
    float3 VrN = PBR.VrN;
    float NdV = PBR.NdV;

    SamplerCube envMap = GLOBALS.IBL_Specular;

    float lod = Surface.Roughness * (GLOBALS.IBL_Levels-1);
    float2 brdfSamplePoint = float2(NdV,Surface.Roughness);
    float2 brdf = SampleBRDFLUT(brdfSamplePoint);
    float3 specularLight = envMap.SampleLevel(VrN,lod).xyz;

    float3 Fr = max(float3(1.0 - Surface.Roughness),
                    Surface.F0) - Surface.F0;
      float3 k_S = Surface.F0 + Fr * pow(1.0-NdV, 5.0);
      float3 FssEss = k_S * brdf.x + brdf.y;
      return SpecularWeight * specularLight * FssEss;
}

float3 CalculatePBRLightContribution(PBRInfo PBR, SurfaceInfo Surface, float3 LightDirection, float3 LightColor)
{
      float3 n = Surface.Normal;
      float3 v = PBR.ViewDir;
      float3 ld = normalize(LightDirection);
      float3 h = normalize(ld +  v);
      float NdotV = PBR.NdV;
      float NdotL = clamp(dot(n, ld), 0.001, 1.0);
      float NdotH = saturate(dot(n, h));
      float LdotH = saturate(dot(ld, h));
      float VdotH = saturate(dot(v, h));
      float3 color = float3(0);

        if (NdotL > 0.0 || NdotV > 0.0) {
          PBR.NdL = NdotL;
          PBR.NdH = NdotH;
          PBR.LdH = LdotH;
          PBR.VdH = VdotH;
          float3  F = FresnelSpecularReflection(PBR,Surface);
          float G = GeometricOcclusion(PBR);
          float D = MicrofacetDistribution(PBR);
    float3 diffuseContrib =  (1 - F)*DiffuseBurley(PBR,Surface);
    float3 specContrib = F * G * D / max((4.0 * NdotL * NdotV),0.0001);
    color = NdotL * LightColor * (diffuseContrib + specContrib);
  }
  return color;
}


public float3 StandardSurface(SurfaceInfo Surface) {

    PBRInfo PBR;
    PBR.NdV = dot(Surface.Normal,Surface.ViewDir);
    PBR.VrN = reflect(-Surface.ViewDir,Surface.Normal);
    PBR.ViewDir = Surface.ViewDir;
    PBR.DiffuseColor = lerp(Surface.Albedo,0, Surface.Metallic);
    PBR.SpecularColor = lerp(Surface.F0, Surface.Albedo,Surface.Metallic);
    PBR.alphaRoughness = Surface.Roughness * Surface.Roughness;
    Surface.F0 = float3(0.2,.2,.2);
    Surface.F90 = float3(1,1.,1.);

    float3 specularColor = GetIBLRadianceContributionGGX(PBR,Surface,1.);
    float3 diffuseColor = GetIBLRadianceDiffuse(PBR,Surface,1.);

    float3 L = CalculatePBRLightContribution(PBR,Surface, normalize(GLOBALS.lightingBuffer.lightDirection.xyz),float3(1,1,1));
    float3 c = specularColor + diffuseColor + Surface.Emissive + L*Surface.ShadowAttenuation;

    switch(DEBUG_SETTINGS.debugMode)
    {
        case DebugMode.Albedo:
            c = Surface.Albedo;
            break;
        case DebugMode.Diffuse:
            c = PBR.DiffuseColor;
            break;
        case DebugMode.Normal:
            c = Surface.Normal * 0.5 + 0.5;
            break;
        case DebugMode.Metallic:
            c = Surface.Metallic;
            break;
        case DebugMode.Roughness:
            c = Surface.Roughness;
            break;
        case DebugMode.Emissive:
            c = Surface.Emissive;
            break;
        case DebugMode.LightDiffuse:
            c = diffuseColor;
            break;
        case DebugMode.LightSpecular:
            c = specularColor;
            break;
        case DebugMode.ShadowAttenuation:
            c = Surface.ShadowAttenuation;
            break;
        case DebugMode.DirectLight:
            c = L;
            break;
            case DebugMode.Irradiance:
                c = SampleIrradiance(Surface.Normal);
                break;
        default:
        break;
    }

    return c;
}
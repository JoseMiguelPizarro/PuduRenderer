import PuduCoreModule;
import PuduRandomModule;
module PuduLightingModule;


#define BlinnPhongSpecularAlpha 50.f

public interface IDiffuseAmbientSampler {
    public float3 Sample(float3 N);
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
    public SamplerCube AmbientSampler;
};

float G_SchlicksmithGGX(
  float DotNL, float DotNV, float Roughness)
{
  float K  = (Roughness * Roughness) / 2.0;
  float GL = DotNL / (DotNL * (1.0 - K) + K);
  float GV = DotNV / (DotNV * (1.0 - K) + K);
  return GL * GV;
}

float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N) {
    float A = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt((1 - Xi.y) / (1 + (A * A - 1) * Xi.y));
    float SinTheta = sqrt(1 - CosTheta * CosTheta);
    float3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;
    float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 TangentX = normalize(cross(UpVector, N));
    float3 TangentY = cross(N, TangentX);
    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 SpecularIBL(SamplerCube EnvMap, float3 SpecularColor, float Roughness, float3 N, float3 V) {
    float3 SpecularLighting = 0;
    const uint NumSamples = 1024;
    for (uint I = 0; I < NumSamples; I++) {
        float2 Xi = HammersleyHemisphere(I, NumSamples);
        float3 H = ImportanceSampleGGX(Xi, Roughness, N);
        float3 L = 2 * dot(V, H) * H - V;
        float Nov = saturate(dot(N, V));
        float Nol = saturate(dot(N, L));
        float Noh = saturate(dot(N, H));
        float Voh = saturate(dot(V, H));
        if (Nol > 0) {
            float3 SampleColor = EnvMap.SampleLevel(L, 0).rgb;
            float G = G_SchlicksmithGGX(Nol, Nov, Roughness);
            float Fc = pow(1 - Voh, 5);
            float3 F = (1 - Fc) * SpecularColor + Fc;
            // Incident light = SampleColor * Nol
            // Microfacet specular = D*G*F / (4*Nol*Nov)
            // pdf = D * Noh / (4 * Voh)
            SpecularLighting += SampleColor * F * G * Voh / (Noh * Nov);
        }
    }
    return SpecularLighting / NumSamples;
}


const static uint K_Num_Samples = 1024;
float3 BRDF(float Ndv, float Roughness) {
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

    return Lut / K_Num_Samples;
}

float3 SampleAmbientSpecular(SurfaceInfo Surface) {
    float3 R = reflect(Surface.ViewDir, Surface.Normal);
    return Surface.AmbientSampler.Sample(R).xyz * (Surface.Glossiness) * .5;
}

float3 SampleAmbientDiffuse(SurfaceInfo Surface) {
    return Surface.AmbientSampler.SampleBias(Surface.Normal, 3.).xyz * (Surface.Glossiness);
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

    return DiffuseReflection + SpecularReflection;
}

public float3 StandardSurface(SurfaceInfo Surface) {
    return LambertBRDF(Surface);
}
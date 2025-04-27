import PuduCoreModule;
module Lighting;


#define BlinnPhongSpecularAlpha 50.f

public interface IDiffuseAmbientSampler
{
    public float3 Sample(float3 n);
}

public struct SurfaceInfo
{
    public float3 albedo;
    public float3 normal;
    public float3 viewDir;
    public float3 lightDir;
    public float3 lightColor;
    public float3 specularColor = float3(1,1,1);
    public float glossiness;
    public float metallic;
    public SamplerCube ambientSampler;
};



float3 SampleAmbientSpecular(SurfaceInfo surface)
{
    float3 r = reflect(surface.viewDir, surface.normal);

    return surface.ambientSampler.Sample(r).xyz * (surface.glossiness)*.5;
}

float3 SampleAmbientDiffuse(SurfaceInfo surface)
{
    return surface.ambientSampler.SampleBias(surface.normal,3.).xyz*(surface.glossiness);
}

float3 BlinnPhongSpecularReflection(SurfaceInfo surface)
{
float3 l = surface.lightDir;
float3 n = surface.normal;
float3 v = surface.viewDir;

float3 h = normalize(l + v);

float3 s = pow( surface.specularColor * surface.lightColor * saturate(dot(n,h)),BlinnPhongSpecularAlpha) * surface.glossiness;
s += SampleAmbientSpecular(surface);

return s;
}
float3 PhongSpecularReflection(SurfaceInfo surface)
{
    float3 l = surface.lightDir;
    float3 n = surface.normal;
    float3 v = surface.viewDir;
    float3 r = reflect(surface.lightDir,surface.normal);

    float3 s = pow(surface.specularColor * surface.lightColor * saturate(dot(v,r)),5 * (surface.glossiness));

    s = s + SampleAmbientSpecular(surface);
    return s;
}

float3 LambertDiffuseReflection(SurfaceInfo surface)
{
    float3 directColor = surface.lightColor * saturate(dot(surface.normal,surface.lightDir));

    return directColor*surface.albedo + SampleAmbientDiffuse(surface); //Missing diffuse ambient color here!
}

float3 LambertBRDF(SurfaceInfo surface)
{
    float3 diffuseReflection = LambertDiffuseReflection(surface);
    float3 specularReflection = BlinnPhongSpecularReflection(surface);

    return diffuseReflection + specularReflection;
}

float3 ToneMap(float3 color, float exposure)
{
    color *= exposure;
    return color / (color + float3(1.0)); // Basic Reinhard tonemapping
}
public float3 StandardSurface(SurfaceInfo surface)
{
    return ToneMap(LambertBRDF(surface),3);
}

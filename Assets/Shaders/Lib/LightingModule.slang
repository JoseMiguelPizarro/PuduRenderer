import PuduCoreModule;
module Lighting;

public struct SurfaceInfo
{
    public float3 albedo;
    public float3 normal;
    public float3 viewDir;
    public float3 lightDir;
    public float3 lightColor;
    public float3 specularColor = float3(1,1,1);
    public float roughness;
    public float metallic;
};

float3 BlinnPhongSpecularReflection(SurfaceInfo mat)
{
float3 l = mat.lightDir;
float3 n = mat.normal;
float3 v = mat.viewDir;

float3 h = normalize(l + v);

float3 s = pow( mat.specularColor * mat.lightColor * saturate(dot(n,h)),mat.roughness);

return s;
}
float3 PhongSpecularReflection(SurfaceInfo mat)
{
    float3 l = mat.lightDir;
    float3 n = mat.normal;
    float3 v = mat.viewDir;
    float3 r = reflect(mat.lightDir,mat.normal);

    float3 s = pow( mat.specularColor * mat.lightColor * saturate(dot(v,r)),5);

    return s;
}

float3 LambertDiffuseReflection(SurfaceInfo mat)
{
    float3 directColor = mat.lightColor * saturate(dot(mat.normal,mat.lightDir));

    return directColor*mat.albedo; //Missing diffuse ambient color here!
}

float3 LambertBRDF(SurfaceInfo mat)
{
    float3 diffuseReflection = LambertDiffuseReflection(mat);
    float3 specularReflection = BlinnPhongSpecularReflection(mat);
    float3 ambient = float3(0.05,0.08,0.07);

    return diffuseReflection + specularReflection + ambient;
}

float3 ToneMap(float3 color, float exposure)
{
    color *= exposure;
    return color / (color + float3(1.0)); // Basic Reinhard tonemapping
}
public float3 StandardSurface(SurfaceInfo mat)
{
    return ToneMap(LambertBRDF(mat),3);
}

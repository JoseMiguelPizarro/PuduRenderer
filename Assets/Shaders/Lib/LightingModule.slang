import PuduCoreModule;
module Lighting;

public struct SurfaceMaterial
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

float3 BlinnPhongSpecularReflection(SurfaceMaterial mat)
{
float3 l = mat.lightDir;
float3 n = mat.normal;
float3 v = mat.viewDir;

float3 h = normalize(l + v);

float3 s = pow( mat.specularColor * mat.lightColor * saturate(dot(n,h)),20);

return s;
}
float3 PhongSpecularReflection(SurfaceMaterial mat)
{
    float3 l = mat.lightDir;
    float3 n = mat.normal;
    float3 v = mat.viewDir;
    float3 r = reflect(mat.lightDir,mat.normal);

    float3 s = pow( mat.specularColor * mat.lightColor * saturate(dot(v,r)),15);

    return s;
}

float3 LambertDiffuseReflection(SurfaceMaterial mat)
{
    float3 directColor = mat.lightColor * saturate(dot(mat.normal,mat.lightDir));

    return directColor*mat.albedo; //Missing diffuse ambient color here!
}

float3 LambertBRDF(SurfaceMaterial mat)
{
    float3 diffuseReflection = LambertDiffuseReflection(mat);
    float3 specularReflection = BlinnPhongSpecularReflection(mat);
    float3 ambient = float3(0.3,0.2,0.1);

    return diffuseReflection + specularReflection + ambient;
}

float3 ToneMap(float3 color, float exposure)
{
    color *= exposure;
    return color / (color + float3(1.0)); // Basic Reinhard tonemapping
}
public float3 StandardSurface(SurfaceMaterial mat)
{
    return ToneMap(LambertBRDF(mat),3.);
}

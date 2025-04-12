import PuduCoreModule;
module Lighting;

struct SurfaceMaterial
{
    float3 albedo;
    float3 normal;
    float3 viewDir;
    float3 lightDir;
    float3 lightColor;
    float roughness;
    float metallic;
};

float3 LambertDiffuseReflection(SurfaceMaterial mat)
{
    float3 directColor = mat.lightColor * saturate(dot(mat.normal,mat.lightDir));

    return directColor*mat.albedo; //Missing diffuse ambient color here!
}

float3 LambertBRDF(SurfaceMaterial mat)
{
    float3 diffuseReflection = LambertDiffuseReflection(mat);

    return diffuseReflection;
}

float3 StandardSurface(SurfaceMaterial mat)
{
    return LambertBRDF(mat);
}

#pragma once
#ifndef PUDU_LIGHTS
#define PUDU_LIGHTS


public struct DirectionalLight
{
    public float4 lightDirection;
    public float4 lightColor;
//Shadow view matrix
    public float4x4 lightMatrix;
//Shadow projection matrix
    public float4x4 shadowMatrix;
};

//TODO: FIX ALIGNMENT
public struct PointLight
{
    public float4 positionAndRange;
    public float4 colorAndIntensity;
};

public struct LightingBuffer
{
    public DirectionalLight directionalLight;
    public int4 lightCount;
    public PointLight[64] pointLight;
};

public struct OmnidirectionalShadowmapData
{
    public  float4 lightCount;
    public float4x4 shadowMatrix[128]; //FOR NOW HARD CODED TO 128. Packed in sets of 4 per each light
};

public struct SceneLighting
{
    public ConstantBuffer<LightingBuffer> lights;
    public Sampler2DShadow shadowMap;
    public Sampler2DShadow omnidirectionalSM;
    public SamplerCube IBL_Specular; //Convoluted image based lighting env map
    public SamplerCube IBL_Diffuse;
    public Sampler2D BRDF_LUT;
    public ConstantBuffer<OmnidirectionalShadowmapData> omnidirectionalData;
    public uint4 IBL_Levels;
};
#endif
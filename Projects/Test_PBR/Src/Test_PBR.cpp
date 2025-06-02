//
// Created by Administrator on 4/12/2025.
//

#include "Test_PBR.h"

#include "FileManager.h"
#include "StringUtils.h"
#include "ImGui/imgui.h"
#include "ComputeShaderRenderer.h"
#include "OverlayQuadEntity.h"

void Test_PBR::OnInit()
{
    TextureLoadSettings hdrSettings{};
    hdrSettings.bindless = false;
    hdrSettings.name = "hdr_sky";
    hdrSettings.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    hdrSettings.generateMipmaps = true;
    hdrSettings.samplerData.wrap = true;

    SPtr<Texture2d> hdrSky = Graphics.LoadTexture2D("textures/skybox/piazza_bologni_4k.ktx2",hdrSettings);

    //Env To Cubemap

    SamplerCreationData samplerCreationData{};
    u32 envCubemapResolution = 1024;
    TextureCreationData envCubemapRTCreationData{};
    envCubemapRTCreationData.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    envCubemapRTCreationData.width = envCubemapResolution;
    envCubemapRTCreationData.height = envCubemapResolution;
    envCubemapRTCreationData.textureType = TextureType::Texture_2D_Array;
    envCubemapRTCreationData.generateMipmaps = false;
    envCubemapRTCreationData.name = "EnvCubeRT";
    envCubemapRTCreationData.flags = TextureFlags::UnorderedAccess;
    envCubemapRTCreationData.samplerData = &samplerCreationData;
    envCubemapRTCreationData.layers = 6;


    auto envCubemapRTHandle = Graphics.CreateTexture(envCubemapRTCreationData);
    auto envCubemapRT = Graphics.Resources()->GetTexture<Texture>(envCubemapRTHandle);

    u32 IBLRTResolution = 1024;
    u32 IBLMips = Texture::CalculateMipLevels(IBLRTResolution,IBLRTResolution);

    TextureCreationData IBLRTCreationData{};
    IBLRTCreationData.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    IBLRTCreationData.width = IBLRTResolution;
    IBLRTCreationData.height = IBLRTResolution;
    IBLRTCreationData.textureType = TextureType::Texture_2D_Array;
    IBLRTCreationData.generateMipmaps = false;
    IBLRTCreationData.name = "IBLRT";
    IBLRTCreationData.flags = TextureFlags::UnorderedAccess;
    IBLRTCreationData.samplerData = &samplerCreationData;
    IBLRTCreationData.layers = 6;
    IBLRTCreationData.exposeMipViews = true;
    IBLRTCreationData.mipmaps = IBLMips;

    auto IBLRTHandle = Graphics.CreateTexture(IBLRTCreationData);
    auto IBLRT = Graphics.Resources()->GetTexture<Texture>(IBLRTHandle);

    u32 brdfLUTResolution = 256;
    SamplerCreationData brdfLUTSamplerCreationData{};
    brdfLUTSamplerCreationData.wrap = false;
    TextureCreationData BRDF_LUTCreationData;
    BRDF_LUTCreationData.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    BRDF_LUTCreationData.width = brdfLUTResolution;
    BRDF_LUTCreationData.height = brdfLUTResolution;
    BRDF_LUTCreationData.generateMipmaps = false;
    BRDF_LUTCreationData.textureType = TextureType::Texture2D;
    BRDF_LUTCreationData.name = "BRDF_LUT";
    BRDF_LUTCreationData.flags = TextureFlags::UnorderedAccess;
    BRDF_LUTCreationData.samplerData = &brdfLUTSamplerCreationData;
    BRDF_LUTCreationData.layers = 1;

    auto BRDF_LUTHandle = Graphics.CreateTexture(BRDF_LUTCreationData);
    auto BRDF_LUT = Graphics.Resources()->GetTexture<Texture>(BRDF_LUTHandle);

    ComputeShaderCreationData BRDF_LUT_CS_Data{ "Compute/brdfLUT.compute.slang", "BRDF_LUT"};
    auto BRDF_LUTCS = Graphics.CreateComputeShader(BRDF_LUT_CS_Data);
    auto BRDF_LUTMat = Graphics.Resources()->AllocateMaterial();

    BRDF_LUTMat->SetShader(BRDF_LUTCS);
    BRDF_LUTMat->SetProperty("material.output", BRDF_LUT);

    ComputeShaderRenderer brdfLutCSRenderer;
    brdfLutCSRenderer.SetShader(BRDF_LUTCS);
    brdfLutCSRenderer.SetMaterial(BRDF_LUTMat);

    Graphics.DispatchCompute(&brdfLutCSRenderer,brdfLUTResolution/32,brdfLUTResolution/32,1);


    ComputeShaderCreationData IBL_ComputeData{ "Compute/IBL.compute.slang", "IBL"};
    ComputeShaderRenderer IBL_CSRenderer;
    auto IBLCS = Graphics.CreateComputeShader(IBL_ComputeData);
    auto IBLMaterial = Graphics.Resources()->AllocateMaterial();
    IBLMaterial->SetShader(IBLCS);
    IBLMaterial->SetProperty("material.input", hdrSky);
    IBLMaterial->SetProperty("material.output", IBLRT);
    IBLMaterial->SetProperty("material.roughness", 1);
    IBLMaterial->SetProperty("material.sampleCount", 1024);

    IBL_CSRenderer.SetShader(IBLCS);
    IBL_CSRenderer.SetMaterial(IBLMaterial);

    Graphics.DispatchCompute(&IBL_CSRenderer,IBLRTResolution/32,IBLRTResolution/32,6);

    TextureCreationData envCubemapCreationData{};
    envCubemapCreationData.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    envCubemapCreationData.width = envCubemapResolution;
    envCubemapCreationData.height = envCubemapResolution;
    envCubemapCreationData.generateMipmaps = false;
    envCubemapCreationData.textureType = TextureType::Texture_Cube;
    envCubemapCreationData.name = "EnvCube";
    envCubemapCreationData.samplerData = &samplerCreationData;
    envCubemapCreationData.layers = 6;

    auto envCubemapHandle = Graphics.CreateTexture(envCubemapCreationData);
    auto envCubeMap = Graphics.Resources()->GetTexture<Texture>(envCubemapHandle);



    ComputeShaderCreationData envToCubemapCS_Data{ "Compute/horizonMapToCubeMap.compute.slang", "horizonToCubemap"};
    auto horizonToCubemapCS = Graphics.CreateComputeShader(envToCubemapCS_Data);
    auto horizonToCubemapMat = Graphics.Resources()->AllocateMaterial();
    horizonToCubemapMat->SetShader(horizonToCubemapCS);
    horizonToCubemapMat->SetProperty("material.output", envCubemapRT);
    horizonToCubemapMat->SetProperty("material.input", hdrSky);

    ComputeShaderRenderer cubeComputeRenderer;
    cubeComputeRenderer.SetShader(horizonToCubemapCS);
    cubeComputeRenderer.SetMaterial(horizonToCubemapMat);
    Graphics.DispatchCompute(&cubeComputeRenderer,envCubemapResolution,envCubemapResolution,6);

    AntialiasingSettings antialiasingSettings{};
    antialiasingSettings.sampleCount = TextureSampleCount::Eight;
    Graphics.SetAntiAliasing(antialiasingSettings);

    m_camera = {};
    m_camera.Transform.SetLocalPosition({0, 0, 6});
    m_camera.Transform.SetForward({0, 0, -1}, {0, 1, 0});
    m_camera.SetClearColor({1, 0, 0, 1});
    Projection projection;

    projection.Width = Graphics.WindowWidth;
    projection.Height = Graphics.WindowHeight;
    projection.Fov = 45;

    m_camera.Projection = projection;

    m_scene = Scene(&Time);
    m_scene.camera = &m_camera;
    TargetFPS = 30;

    m_puduRenderer.Init(&Graphics, this);

    auto cmd = Graphics.BeginSingleTimeCommands();
        cmd.Blit(IBLRT,envCubeMap);
    Graphics.EndSingleTimeCommands(cmd);

    standardShader = Graphics.CreateShader("standardSurface.shader.slang", "standard");
    TextureLoadSettings settings{};
    settings.bindless = false;
    settings.name = "stringy_marble_albedo";
    settings.format = VK_FORMAT_R8G8B8A8_UNORM;
    settings.generateMipmaps = true;
    settings.samplerData.wrap = true;


    SPtr<Texture2d> albedoTexture = Graphics.LoadTexture2D("textures/patched-brickwork/patched-brickwork_albedo.png",
                                                           settings);
    SPtr<Texture2d> normalTexture = Graphics.LoadTexture2D(
        "textures/patched-brickwork/patched-brickwork_Normal-ogl.png", settings);
    SPtr<Texture2d> roughnessTexture = Graphics.LoadTexture2D(
        "textures/patched-brickwork/patched-brickwork_roughness.png", settings);
    SPtr<Texture2d> heightTexture = Graphics.LoadTexture2D("textures/patched-brickwork/patched-brickwork_height.png",
                                                           settings);

    projection.nearPlane = 5;
    projection.farPlane = 50;
    directionalLight = {};
    directionalLight.Projection = projection;
    directionalLight.GetTransform().SetForward({1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f});
    directionalLight.GetTransform().SetLocalPosition({20, 20, 20});
    m_scene.directionalLight = &directionalLight;


    //SKYBOX
    auto sphere = FileManager::LoadGltfScene("models/sphere.gltf");

    const auto skyboxModel = std::dynamic_pointer_cast<RenderEntity>(FileManager::LoadGltfScene("models/skybox.gltf"));

    auto skyboxShader = Graphics.CreateShader("skybox.shader.slang", "skybox");
    const auto skyboxMaterial = skyboxModel->GetModel()->Materials[0];
    skyboxMaterial->name = "Skybox";
    skyboxMaterial->SetShader(skyboxShader);
    skyboxMaterial->SetProperty("material.skyboxTex", envCubeMap);

    auto sphereEntity = std::dynamic_pointer_cast<RenderEntity>(sphere);
    auto material = sphereEntity->GetModel()->Materials[0];
    material->SetShader(standardShader);
    material->SetProperty("material.albedoTex", albedoTexture);
    material->SetProperty("material.normalTex", normalTexture);
    material->SetProperty("material.roughnessTex", roughnessTexture);
    material->SetProperty("material.heightTex", heightTexture);
    material->SetProperty("material.skybox", envCubeMap);

    skyboxModel->GetTransform().SetUniformLocalScale(80);

    const auto overlayShader = Graphics.CreateShader("overlay.slang", "overlay");
    const auto axisModel = std::dynamic_pointer_cast<RenderEntity>(FileManager::LoadGltfScene("models/axis.gltf"));
    axisModel->GetTransform().SetLocalPosition({0, 0, 0});
    axisModel->GetTransform().SetUniformLocalScale(0.2f);
    auto& [layer] = axisModel->GetRenderSettings();
    layer = 2;
    axisModel->GetModel()->Materials[0]->SetShader(overlayShader);

    m_scene.AddEntity(sphere);
    m_scene.AddEntity(skyboxModel);
    m_scene.AddEntity(axisModel);


    auto inputQO = std::make_shared<OverlayQuadEntity>(OverlayQuadEntity(&Graphics));
    inputQO->GetMaterial()->SetProperty("material.texture", hdrSky);
    float qoSize = 0.15;

    inputQO->SetPositionAndSize(0.0, 0.0, qoSize*2., qoSize);
    inputQO->SetPtr(inputQO);

    auto oq = std::make_shared<OverlayQuadEntity>(OverlayQuadEntity(&Graphics));
    oq->GetMaterial()->SetProperty("material.texture", BRDF_LUT);
    oq->SetPositionAndSize(0.0,qoSize*1.1,0.15,0.15);
    oq->SetPtr(oq);

    m_arrayQO = std::make_shared<OverlayQuadTextureArrayEntity>(OverlayQuadTextureArrayEntity(&Graphics));
    m_arrayQO->GetMaterial()->SetProperty("material.texture", IBLRT);
    m_arrayQO->SetPositionAndSize(qoSize*1.1,qoSize*1.1,.15,.15);
    m_arrayQO->SetTextureIndex(0);
    m_arrayQO->SetLOD(0);

    m_arrayQO->SetPtr(m_arrayQO);

    m_scene.AddEntity(inputQO);
    m_scene.AddEntity(oq);
    m_scene.AddEntity(m_arrayQO);
}

void Test_PBR::OnRun()
{
    m_puduRenderer.Render(&m_scene);
    static float angle = PI/4 + PI;
    const float radius = 3.5f;
    const float speed = 0.0001f; // radians per frame

    // Update the angle based on speed
    angle += speed * Time.DeltaTime();

    // Calculate the new position of the camera
    float x = radius * cos(angle);
    float z = radius * sin(angle);
    float y = sin(angle)*radius;

    // Set the camera position and keep it above the XZ plane (upper hemisphere)
    m_camera.Transform.SetLocalPosition({x, y, z});

    // Make the camera look at the origin
    m_camera.Transform.SetForward(-m_camera.Transform.GetLocalPosition(), {0.0f, 1.0f, 0.0f});
}

void Test_PBR::DrawImGUI()
{
    PuduApp::DrawImGUI();

    ImGui::Text(StringUtils::Format("FPS: {}", Time.GetFPS()).c_str());
    ImGui::Text(StringUtils::Format("Time: {}", Time.Time()).c_str());
    ImGui::Text(StringUtils::Format("DeltaTime: {}", Time.DeltaTime()).c_str());

    static int index = 0;

    if (ImGui::SliderInt("Array", &index,0,5))
    {
        m_arrayQO->SetTextureIndex(index);
    }

    static int lod = 0;
    if (ImGui::SliderInt("Lod",&lod,0,11))
    {
        m_arrayQO->SetLOD(lod);
    }
}

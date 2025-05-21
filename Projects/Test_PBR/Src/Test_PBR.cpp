//
// Created by Administrator on 4/12/2025.
//

#include "Test_PBR.h"

#include "FileManager.h"
#include "StringUtils.h"
#include "ImGui/imgui.h"
#include "ComputeShaderRenderer.h"
#include "OverlayQuadEntity.h"

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

    // Set the camera position and keep it above the XZ plane (upper hemisphere)
    m_camera.Transform.SetLocalPosition({x, .3f, z});

    // Make the camera look at the origin
    m_camera.Transform.SetForward(-m_camera.Transform.GetLocalPosition(), {0.0f, 1.0f, 0.0f});

}

void Test_PBR::OnInit()
{
    auto testCompute = Graphics.CreateComputeShader("Compute/Test.compute.slang", "testCompute");

    auto testRT = Graphics.GetRenderTexture();
    testRT->depth =1;
    testRT->width = 4096;
    testRT->height = 2048;
    testRT->format = VK_FORMAT_R8G8B8A8_UNORM;
    testRT->name ="TestRT";
    testRT->SetUsage(ResourceUsage::UNORDERED_ACCESS);
    testRT->Create(&Graphics);

    TextureLoadSettings hdrSettings{};
    hdrSettings.bindless = false;
    hdrSettings.name = "hdr_sky";
    hdrSettings.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    hdrSettings.generateMipmaps = false;
    hdrSettings.samplerData.wrap = true;

    SPtr<Texture2d> hdrSky = Graphics.LoadTexture2D("textures/skybox/kloofendal_48d_partly_cloudy_puresky_4k.ktx2",hdrSettings);

    auto computeMaterial = Graphics.Resources()->AllocateMaterial();
    computeMaterial->SetShader(testCompute);
    computeMaterial->SetProperty("material.input", hdrSky);
    computeMaterial->SetProperty("material.output", testRT);

    ComputeShaderRenderer computeRenderer;
    computeRenderer.SetShader(testCompute);
    computeRenderer.SetMaterial(computeMaterial);

    Graphics.DispatchCompute(&computeRenderer,4096,2048,1);

    AntialiasingSettings antialiasingSettings{};
    antialiasingSettings.sampleCount = TextureSampleCount::Eight;
    Graphics.SetAntiAliasing(antialiasingSettings);

    m_camera = {};
    m_camera.Transform.SetLocalPosition({0, 0, -6});
    m_camera.Transform.SetForward({0, 0, 1}, {0, 1, 0});
    m_camera.SetClearColor({1, 0, 0, 1});
    Projection projection;

    projection.Width = Graphics.WindowWidth;
    projection.Height = Graphics.WindowHeight;
    projection.Fov = 45;

    m_camera.Projection = projection;

    m_scene = Scene(&Time);
    m_scene.camera = &m_camera;
    TargetFPS = 120;

    m_puduRenderer.Init(&Graphics, this);

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

    auto sphere = FileManager::LoadGltfScene("models/sphere.gltf");


    TextureLoadSettings skyTexSettings{};
    skyTexSettings.bindless = false;
    skyTexSettings.name = "Sky";
    skyTexSettings.format = VK_FORMAT_R8G8B8A8_UNORM;
    skyTexSettings.textureType = TextureType::Texture_Cube;

    const auto skyTexture = Graphics.LoadTextureCube("textures/skyCube.ktx", skyTexSettings);
    const auto skyboxModel = std::dynamic_pointer_cast<RenderEntity>(FileManager::LoadGltfScene("models/skybox.gltf"));

    auto skyboxShader = Graphics.CreateShader("skybox.shader.slang", "skybox");
    const auto skyboxMaterial = skyboxModel->GetModel()->Materials[0];
    skyboxMaterial->name = "Skybox";
    skyboxMaterial->SetShader(skyboxShader);
    skyboxMaterial->SetProperty("material.skyboxTex", skyTexture);

    auto sphereEntity = std::dynamic_pointer_cast<RenderEntity>(sphere);
    auto material = sphereEntity->GetModel()->Materials[0];
    material->SetShader(standardShader);
    material->SetProperty("material.albedoTex", albedoTexture);
    material->SetProperty("material.normalTex", normalTexture);
    material->SetProperty("material.roughnessTex", roughnessTexture);
    material->SetProperty("material.heightTex", heightTexture);
    material->SetProperty("material.skybox", skyTexture);

    skyboxModel->GetTransform().SetUniformLocalScale(80);

    const auto overlayShader = Graphics.CreateShader("overlay.slang", "overlay");
    const auto axisModel = std::dynamic_pointer_cast<RenderEntity>(FileManager::LoadGltfScene("models/axis.gltf"));
    axisModel->GetTransform().SetLocalPosition({0, 0, 0});
    axisModel->GetTransform().SetUniformLocalScale(0.2f);
    auto& [layer] = axisModel->GetRenderSettings();
    layer = 2;
    axisModel->GetModel()->Materials[0]->SetShader(overlayShader);

    Graphics.GetDefaultOverlayShader()->GetShaderLayout()->Print();
    m_scene.AddEntity(sphere);
    m_scene.AddEntity(skyboxModel);
    m_scene.AddEntity(axisModel);

    auto oq = std::make_shared<OverlayQuadEntity>(OverlayQuadEntity(&Graphics));

    auto inputQO = std::make_shared<OverlayQuadEntity>(OverlayQuadEntity(&Graphics));
    inputQO->GetMaterial()->SetProperty("material.texture", hdrSky);
    float qoSize = 0.15;

    inputQO->SetPositionAndSize(0.0, 0.0, qoSize, qoSize);
    inputQO->SetPtr(inputQO);

    oq->GetMaterial()->SetProperty("material.texture", testRT);
    oq->SetPositionAndSize(0.0,qoSize*1.1,qoSize,qoSize);
    oq->SetPtr(oq);

    m_scene.AddEntity(inputQO);
    m_scene.AddEntity(oq);
}

void Test_PBR::DrawImGUI()
{
    PuduApp::DrawImGUI();

    ImGui::Text(StringUtils::Format("FPS: {}", Time.GetFPS()).c_str());
    ImGui::Text(StringUtils::Format("Time: {}", Time.Time()).c_str());
    ImGui::Text(StringUtils::Format("DeltaTime: {}", Time.DeltaTime()).c_str());
}

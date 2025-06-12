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

    standardShader = Graphics.GetDefaultStandardShader();
    TextureLoadSettings settings{};
    settings.bindless = false;
    settings.name = "stringy_marble_albedo";
    settings.format = VK_FORMAT_R8G8B8A8_UNORM;
    settings.generateMipmaps = true;
    settings.samplerData.wrap = true;

    TextureLoadSettings skyboxLoadSettings{};
    skyboxLoadSettings.bindless = false;
    skyboxLoadSettings.name = "Skybox";
    skyboxLoadSettings.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    skyboxLoadSettings.samplerData.wrap = true;
    skyboxLoadSettings.generateMipmaps = true;

    auto skybox = Graphics.LoadTextureHorizonAsCube("textures/skybox/piazza_bologni_4k.ktx2", skyboxLoadSettings);
    m_puduRenderer.SetSkyBox(skybox);

    SPtr<Texture2d> albedoTexture = Graphics.LoadTexture2D("textures/patched-brickwork/patched-brickwork_albedo.png",
                                                           settings);
    SPtr<Texture2d> normalTexture = Graphics.LoadTexture2D(
        "textures/patched-brickwork/patched-brickwork_Normal-ogl.png", settings);
    SPtr<Texture2d> roughnessTexture = Graphics.LoadTexture2D(
        "textures/patched-brickwork/patched-brickwork_roughness.png", settings);
    SPtr<Texture2d> heightTexture = Graphics.LoadTexture2D("textures/patched-brickwork/patched-brickwork_height.png",
                                                           settings);


    SPtr<Texture2d> silverAlbedoTexture = Graphics.LoadTexture2D("textures/silver-bl/silver_albedo.png", settings);
    SPtr<Texture2d> silverNormalTexture = Graphics.LoadTexture2D("textures/silver-bl/silver_normal-ogl.png", settings);
    SPtr<Texture2d> silverRoughnessTexture = Graphics.
        LoadTexture2D("textures/silver-bl/silver_roughness.png", settings);
    SPtr<Texture2d> silverMetalnessTexture = Graphics.
        LoadTexture2D("textures/silver-bl/silver_metallic.png", settings);
    SPtr<Texture2d> silverHeightTexture = Graphics.LoadTexture2D("textures/silver-bl/silver_height.png", settings);

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

    auto sphereEntity = std::dynamic_pointer_cast<RenderEntity>(sphere);
    auto material = sphereEntity->GetModel()->Materials[0];
    material->SetShader(standardShader);
    material->SetProperty("material.albedoTex", silverAlbedoTexture);
    material->SetProperty("material.normalTex", Graphics.GetDefaultNormalMapTexture());
    material->SetProperty("material.metallicRoughnessTex", Graphics.GetDefaultMetallicRoughnessTexture());
    material->SetProperty("material.heightTex", silverHeightTexture);

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
    inputQO->GetMaterial()->SetProperty("material.texture", m_puduRenderer.GetEnvMap());
    float qoSize = 0.15;

    inputQO->SetPositionAndSize(0.0, 0.0, qoSize * 2., qoSize);
    inputQO->SetPtr(inputQO);

    auto oq = std::make_shared<OverlayQuadEntity>(OverlayQuadEntity(&Graphics));
    oq->GetMaterial()->SetProperty("material.texture", m_puduRenderer.GetBRDF_LUT());
    oq->SetPositionAndSize(0.0, qoSize * 1.1, 0.15, 0.15);
    oq->SetPtr(oq);

    m_arrayQO = std::make_shared<OverlayQuadTextureArrayEntity>(OverlayQuadTextureArrayEntity(&Graphics));
    m_arrayQO->GetMaterial()->SetProperty("material.texture", m_puduRenderer.GetIBLDiffuse());
    m_arrayQO->SetPositionAndSize(qoSize * 1.1, qoSize * 1.1, .15, .15);
    m_arrayQO->SetTextureIndex(0);
    m_arrayQO->SetLOD(0);
    m_arrayQO->SetPtr(m_arrayQO);

  //  m_scene.AddEntity(inputQO);
    m_scene.AddEntity(oq);
    // m_scene.AddEntity(m_arrayQO);
}

void Test_PBR::OnRun()
{
    m_puduRenderer.Render(&m_scene);
    static float angle = PI / 4 + PI;
    const float radius = 3.5f;
    const float speed = 0.0001f; // radians per frame

    // Update the angle based on speed
    angle += speed * Time.DeltaTime();

    // Calculate the new position of the camera
    float x = radius * cos(angle);
    float z = radius * sin(angle);
    float y = sin(angle) * radius;

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

    if (ImGui::SliderInt("Array", &index, 0, 5))
    {
        m_arrayQO->SetTextureIndex(index);
    }

    static int lod = 0;
    if (ImGui::SliderInt("Lod", &lod, 0, 11))
    {
        m_arrayQO->SetLOD(lod);
    }
}

//
// Created by Administrator on 4/12/2025.
//

// Forward declare post processing functions
void SetPostProcessingEnabled(bool enabled);

#include "Test_PBR.h"
#include "Renderer.h"

#include "FileManager.h"
#include "StringUtils.h"
#include "ImGui/imgui.h"
#include "ComputeShaderRenderer.h"
#include "ImGuiUtils.h"
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
    directionalLight.GetTransform().SetLocalPosition({-20, 20, -20});
    m_scene.directionalLight = &directionalLight;


    //SKYBOX
    auto spheregltf = FileManager::LoadGltfScene("models/sphere.gltf");
    auto sphere = spheregltf->GetChildByName<RenderEntity>("Sphere.001");

    const auto skyboxModel = FileManager::LoadGltfScene("models/skybox.gltf")->GetChildByName<RenderEntity>("Sphere");

    auto skyboxShader = Graphics.CreateShader("skybox.shader.slang", "skybox");
    const auto skyboxMaterial = skyboxModel->GetModel()->Materials[0];
    skyboxMaterial->name = "Skybox";
    skyboxMaterial->SetShader(skyboxShader);

    skyboxModel->GetTransform().SetUniformLocalScale(80);

    m_model = FileManager::LoadGltfScene("models/damagedHelmet/damagedHelmet.gltf");

    const auto overlayShader = Graphics.CreateShader("overlay.slang", "overlay");
    const auto axisModel = FileManager::LoadGltfScene("models/axis.gltf")->GetChildByName<RenderEntity>("OUT_AXIS");
    axisModel->GetTransform().SetLocalPosition({0, 0, 0});
    axisModel->GetTransform().SetUniformLocalScale(0.2f);
    auto& [layer] = axisModel->GetRenderSettings();
    layer = 2;
    axisModel->GetModel()->Materials[0]->SetShader(overlayShader);

    m_model->GetTransform().SetLocalPosition({0, 0, 0});
    m_model->GetTransform().SetUniformLocalScale(.5);
    // m_scene.AddEntity(sphere);
    m_scene.AddEntity(m_model);
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

    m_scene.AddEntity(oq);
}

void Test_PBR::OnRun()
{
    m_puduRenderer.Render(&m_scene);

    m_camRadius -= Input::GetMouseWheelDelta() / 10.f;
    static float angle = PI / 4 + PI;

    static float phi = 0.f;
    static float theta = 0.f;
    if (Input::IsMouseButtonPressed(MouseButton::Left))
    {
        const auto mousePos = Input::GetMousePosition();
        const auto mousePosNormalized = mousePos / glm::vec2{Graphics.WindowWidth, Graphics.WindowHeight};
        const auto mousePosNormalized2 = mousePosNormalized * 2.f - 1.f;

        auto delta = Input::GetMousePositionDelta();
        phi += delta.x * 0.01f;
        theta += delta.y * 0.01f;
    }

    const float radius = m_camRadius;


    float x = radius * cos(phi);
    float z = radius * sin(phi);
    float y = sin(theta) * radius;

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

    using namespace StringUtils;
    auto entities = m_scene.GetEntities();

    ImGuiUtils::DrawEntityTree(entities);

    auto textures = Graphics.Resources()->GetAllocatedTextures()->GetAllResources();

    if (ImGui::CollapsingHeader("Textures"))
    {
        if (ImGui::BeginTable("textures", 2))
        {
            for (size_t row = 0; row < textures.size(); row++)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text(textures[row]->name.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d", textures[row]->Handle().Index());
            }

            ImGui::EndTable();
        }
    }

    ImGuiUtils::DrawShaderTree(&Graphics, Graphics.Resources()->GetAllocatedShaders()->GetAllResources());

    if (ImGui::CollapsingHeader("Materials"))
    {
        const auto materials = Graphics.Resources()->GetAllocatedMaterials()->GetAllResources();

        for (Size row = 0; row < materials.size(); row++)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(materials[row]->name.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", materials[row]->Handle().Index());
        }
    }


    ImGui::Text("Camera");
    ImGuiUtils::DrawTransform(m_camera.Transform);
    ImGui::Text("Projection");
    ImGuiUtils::DrawMat4x4(m_camera.Projection.GetProjectionMatrix());

    static bool postProcessingEnabled = true;
    if (ImGui::Checkbox("Post Processing", &postProcessingEnabled))
        m_puduRenderer.EnablePostProcessing(postProcessingEnabled);


    ImGui::Text("Light");
    vec3 forward = directionalLight.GetTransform().GetForward();
    if (ImGui::InputFloat3("Light Direction", &forward[0]))
    {
        directionalLight.GetTransform().SetForward(normalize(forward), {0, 1, 0});
    }

    static Renderer::Debug currentDebugMode = Renderer::Debug::None;
    //None,Albedo,Diffuse,Normal, Metallic, Roughness, Emissive
    const char* debugModeNames[] = {
        "None", "Albedo", "Diffuse", "Normal", "Metallic", "Roughness", "Emissive", "LightSpecular", "LightDiffuse",
        "ShadowAttenuation", "DirectLight", "Irradiance", "BRDFLUT"
    };

    if (ImGui::Combo("Debug Mode", reinterpret_cast<int*>(&currentDebugMode), debugModeNames,
                     IM_ARRAYSIZE(debugModeNames)))
    {
        m_puduRenderer.SetDebugMode(currentDebugMode);
    }

    static vec3 F0 = {0.04f, 0.04f, 0.04f};
    static vec3 F90 = {0.5f, 0.5f, 0.5f};
    if (ImGui::InputFloat3("F0", &F0[0]))
    {
        auto modelMat = m_model->GetChildByName<RenderEntity>("mesh_helmet_LP_13930damagedHelmet")->GetModel()->
                                 Materials[0];
        modelMat->SetProperty("material.F0", F0);
    }
    if (ImGui::InputFloat3("F90", &F90[0]))
    {
        auto modelMat = m_model->GetChildByName<RenderEntity>("mesh_helmet_LP_13930damagedHelmet")->GetModel()->
                                 Materials[0];
        modelMat->SetProperty("material.F90", F90);
    }

    static float roughnessScale = 1.0f;
    if (ImGui::SliderFloat("Roughness Scale", &roughnessScale, 0.0f, 10.0f))
    {
        m_puduRenderer.SetRoughnessScale(roughnessScale);
    }

    static float modelScale = 1.0f;
    if (ImGui::SliderFloat("Model Scale", &modelScale, 0.0f, 20.0f))
    {
        m_model->GetTransform().SetUniformLocalScale(modelScale);
        m_model->GetTransform().UpdateWorldTransformRecursivelly();
    }

    static float gamma = 2.2f;
    if (ImGui::SliderFloat("Gamma", &gamma, 1.0f, 3.0f))
    {
        m_puduRenderer.SetGamma(gamma);
    }
}

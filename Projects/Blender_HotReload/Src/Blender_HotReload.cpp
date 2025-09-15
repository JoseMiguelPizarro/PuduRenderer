//
// Created by Hojaverde on 4/12/2025.
//

#include "Blender_HotReload.h"
#include "Renderer.h"
#include "FileManager.h"
#include "FileWatch.hpp"
#include "ImGuiUtils.h"
#include "GPUEnums.h"
#include "OverlayQuadEntity.h"

std::unique_ptr<filewatch::FileWatch<std::string>> watch;
namespace Pudu
{
void Blender_HotReload::OnInit()
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

    m_puduRenderer.Init(&Graphics, this);

    projection.nearPlane = 5;
    projection.farPlane = 20;
    projection.Fov = 10.;

    TextureLoadSettings skyboxLoadSettings{};
    skyboxLoadSettings.bindless = false;
    skyboxLoadSettings.name = "Skybox";
    skyboxLoadSettings.format = ToVk(ChannelFormat::R32G32B32A32_SFLOAT);
    skyboxLoadSettings.samplerData.wrap = true;
    skyboxLoadSettings.generateMipmaps = true;

    auto skybox = Graphics.LoadTextureHorizonAsCube("textures/skybox/piazza_bologni_4k.ktx2", skyboxLoadSettings);
    m_puduRenderer.SetSkyBox(skybox, true);

    const auto skyboxModel = FileManager::LoadGltfScene("models/skybox.gltf")->GetChildByName<RenderEntity>("Sphere");

    auto skyboxShader = Graphics.CreateShader("skybox.shader.slang", "skybox");
    const auto skyboxMaterial = skyboxModel->GetModel()->Materials[0];
    skyboxMaterial->name = "Skybox";
    skyboxMaterial->SetShader(skyboxShader);

    skyboxModel->GetTransform().SetUniformLocalScale(80);

    auto blenderFilePath = "models/test.blend";
    m_model = FileManager::LoadGltfScene("models/testoutput.glb");

    watch = std::make_unique<filewatch::FileWatch<std::string>>(
        fs::absolute(blenderFilePath).string().c_str(), [this](const filewatch::CallbackInformation<std::string>& info)
        {
            if (info.event == filewatch::Event::modified || info.event == filewatch::Event::renamed_old)
            {
                std::filesystem::path script = R"(gltfexport.bat)";
                std::string command =std::format("cd models &&{}",script.string().c_str());

                std::system(command.c_str());

                m_scene.RemoveEntity(m_model);
                m_model = nullptr;

                m_model = FileManager::LoadGltfScene("models/testoutput.glb");

                m_scene.AddEntity(m_model);
            }
        });

    auto overlay = std::make_shared<OverlayQuadEntity>(&Graphics);

    overlay->SetPositionAndSize(0, 0, .3,.3);
    overlay->SetTexture(m_puduRenderer.GetOmnidirectionalShadowmapRT());

    m_scene.AddEntity(skyboxModel);
    m_scene.AddEntity(m_model);
    m_scene.AddEntity(overlay);
}

void Blender_HotReload::OnRun()
{
    m_puduRenderer.Render(&m_scene);

    m_camRadius -= Input::GetMouseWheelDelta() / 10.f;
    static float angle = PI / 4 + PI;

    static float phi = 0.f;
    static float theta = 0.f;
    if (Input::IsMouseButtonPressed(MouseButton::Left))
    {
        const auto mousePos = Input::GetMousePosition();
        const auto mousePosNormalized = mousePos / vec2{Graphics.WindowWidth, Graphics.WindowHeight};
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

void Blender_HotReload::DrawImGUI()
{
    PuduApp::DrawImGUI();

    auto entities = m_scene.GetEntities();
    ImGuiUtils::DrawEntityTree(m_scene.GetEntities());

    ImGui::Text(std::format("FPS: {}", Time.GetFPS()).c_str());
    ImGui::Text(std::format("Time: {}", Time.Time()).c_str());
    ImGui::Text(std::format("DeltaTime: {}", Time.DeltaTime()).c_str());

    ImGuiUtils::DrawShaderTree(&Graphics, Graphics.Resources()->GetAllocatedShaders()->GetAllResources());

        static bool postProcessingEnabled = true;
    if (ImGui::Checkbox("Post Processing", &postProcessingEnabled))
        m_puduRenderer.EnablePostProcessing(postProcessingEnabled);

    static bool toneMappingEnabled = true;
    if (ImGui::Checkbox("Tone Mapping", &toneMappingEnabled))
    {
        m_puduRenderer.EnableToneMapping(toneMappingEnabled);
    }

    static float exposure = 1.0f;
    if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 10.0f))
    {
        m_puduRenderer.SetExposure(exposure);
    }

    ImGui::Text("Light");

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

    ImGui::Text("Shadows");
    static float shadowBias;
    static float shadowSlopeBias;
    if (ImGui::SliderFloat("ShadowBias", &shadowBias, 0.0f, 10.f)
        || ImGui::SliderFloat("ShadowSlopeBias", &shadowSlopeBias, 0.0f, 10.f))
    {
        m_puduRenderer.SetShadowBias(shadowSlopeBias, shadowBias);
    }

    // auto projection = m_directionalLight->GetProjection();
    // ImGui::SliderFloat("FoV", &projection.Fov, 0.0f, 100.f);
    // ImGui::SliderFloat("Near", &projection.nearPlane, 0.0f, 100.f);
    // ImGui::SliderFloat("Far", &projection.farPlane, 0.0f, 100.f);
    // m_directionalLight->SetProjection(projection);
}
}


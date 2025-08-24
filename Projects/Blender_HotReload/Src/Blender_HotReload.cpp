//
// Created by Hojaverde on 4/12/2025.
//

#include "Blender_HotReload.h"
#include "Renderer.h"
#include "FileManager.h"
#include "FileWatch.hpp"
#include "ImGuiUtils.h"
#include "GPUEnums.h"

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
    TargetFPS = 30;

    m_puduRenderer.Init(&Graphics, this);

    projection.nearPlane = 5;
    projection.farPlane = 20;
    projection.Fov = 10.;
    m_directionalLight = {};
    m_directionalLight.Projection = projection;

    m_directionalLight.GetTransform().SetForward({1.0f, -1, 1}, {0.0f, 1.0f, 0.0f});
    vec3 lightDirection = -m_directionalLight.GetTransform().GetForward();

    m_directionalLight.GetTransform().SetLocalPosition({
        lightDirection.x * m_lightDistance, lightDirection.y * m_lightDistance, lightDirection.z * m_lightDistance
    });
    m_scene.directionalLight = &m_directionalLight;

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

    m_scene.AddEntity(skyboxModel);
    m_scene.AddEntity(m_model);
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
}
}


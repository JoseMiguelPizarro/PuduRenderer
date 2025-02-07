#include <fmt/core.h>

#include <FileManager.h>
#include "CatDiorama.h"
#include "EntityManager.h"

namespace fs = std::filesystem;

void CatDiorama::OnRun()
{
    m_puduRenderer.Render(&m_scene);

    float radius = 20;
    float pich = 45;

    float speed = 0.15f;
    float phase = Time.Time() * speed;

    float x = cos(phase) * radius;
    float z = sin(phase) * radius;
    float y = sin(glm::radians(30.f)) * radius;

    vec3 pos = vec3(x, y, z);
    m_camera.Transform.SetLocalPosition(pos);

    vec3 forward = glm::normalize(vec3(0) - pos);

    m_camera.Transform.SetForward(forward, {0, 1, 0});
}

void CatDiorama::OnInit()
{
    m_camera = {};
    m_camera.Transform.SetForward(vec3(0, -.8, -1), vec3(0, 1, 0));
    m_camera.Transform.SetLocalPosition({0, 14.0f, 20.0f});
    Projection projection;

    projection.Width = Graphics.WindowWidth;
    projection.Height = Graphics.WindowHeight;
    projection.Fov = 45;

    m_camera.Projection = projection;

    m_scene = Scene(&Time);
    m_scene.camera = &m_camera;
    TargetFPS = 30;

    m_puduRenderer.Init(&Graphics, this);

    standardShader = Graphics.CreateShader("standard.shader.slang", "standard");

    projection.nearPlane = 5;
    projection.farPlane = 50;
    directionalLight = {};
    directionalLight.Projection = projection;
    directionalLight.GetTransform().SetForward({-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f});
    directionalLight.GetTransform().SetLocalPosition({20, 20, 20});
    m_scene.directionalLight = &directionalLight;

   // LoadGameboyModel();

    TextureLoadSettings planeSettings{};
    planeSettings.bindless = false;
    planeSettings.name = "Plane";

    TextureLoadSettings cubemapSettings{};
    cubemapSettings.bindless = false;
    cubemapSettings.name = "Cubemap";
    cubemapSettings.format = VK_FORMAT_R8G8B8A8_UNORM;

    m_cubemapTexture = Graphics.LoadTextureCube(cubeMapPath, cubemapSettings);
    const auto transParentShader = Graphics.CreateShader("transparent.slang", "transparent");

    const auto cube = FileManager::LoadGltfScene("models/sphere.gltf");
    const auto axisModel = FileManager::LoadGltfScene("models/axis.gltf");
    const auto overlayShader = Graphics.CreateShader("overlay.slang", "overlay");

    const auto catModel = FileManager::LoadGltfScene("models/Diorama_Cat/cat.gltf");


    auto planeMaterial = Graphics.Resources()->AllocateMaterial();
    planeMaterial->name ="PlaneMat";
    planeMaterial->SetShader(standardShader);
    planeMaterial->SetProperty("material.baseColorTex",Graphics.GetDefaultWhiteTexture());

    auto planeModel = Graphics.CreateModel(Graphics.GetDefaultQuad(), planeMaterial);


    auto planeName = std::string("Plane");
    auto planeEntity = EntityManager::AllocateRenderEntity(planeName, planeModel);
    planeEntity->GetTransform().SetLocalPosition({0, 0, 0});
    planeEntity->GetTransform().SetLocalScale({100, 100, 100});

    m_scene.AddEntity(planeEntity);

    for (const auto& e : axisModel)
    {
        RenderEntitySPtr re = std::dynamic_pointer_cast<RenderEntity>(e);
        re->GetTransform().SetLocalPosition({0, 0, 0});
        //re->GetTransform().SetLocalScale({ 3,3,3 });
        auto& [layer] = re->GetRenderSettings();
        layer = 2;

        if (re != nullptr)
        {
            auto mat = re->GetModel()->Materials[0];
            mat->SetShader(overlayShader);
        }
    }

    for (const auto& e : catModel)
    {
        RenderEntitySPtr re = std::dynamic_pointer_cast<RenderEntity>(e);
        re->GetTransform().SetLocalScale({3, 3, 3});
        re->GetTransform().SetLocalPosition({0, 1.5, 0});
        if (re != nullptr)
        {
            const auto mat = re->GetModel()->Materials[0];
            mat->SetShader(standardShader);
        }
    }

    for (const auto& e : cube)
    {
        RenderEntitySPtr re = std::dynamic_pointer_cast<RenderEntity>(e);
        re->GetTransform().SetLocalPosition({0, 5, 0});
        re->GetTransform().SetLocalScale({3, 3, 3});
        auto& [layer] = re->GetRenderSettings();
        layer = 1;

        if (re != nullptr)
        {
            const auto mat = re->GetModel()->Materials[0];
            mat->SetShader(transParentShader);
        }
    }

    m_scene.AddEntities(axisModel);
    //m_scene.AddEntities(cube);
    m_scene.AddEntities(catModel);
}

void CatDiorama::DrawImGUI()
{
    auto entities = m_scene.GetEntities();
    ImGui::Text(fmt::format("Time: {}", Time.Time()).c_str());
    ImGui::Text(fmt::format("FPS: {}", Time.GetFPS()).c_str());
    ImGui::Text(fmt::format("DeltaTime: {}", Time.DeltaTime()).c_str());

    ImGuiUtils::DrawEntityTree(entities);

    auto textures = Graphics.Resources()->GetAllocatedTextures()->GetAllResources();

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

void CatDiorama::OnCleanup()
{

}

void CatDiorama::LoadGameboyModel()
{
    auto gltfScene = FileManager::LoadGltfScene(GameboyModelPath);

    for (auto& e : gltfScene)
    {
        RenderEntitySPtr re = std::dynamic_pointer_cast<RenderEntity>(e);

        if (re != nullptr)
        {
            re->GetModel()->Materials[0]->SetShader(standardShader);
        }
    }

    m_scene.AddEntities(gltfScene);
}

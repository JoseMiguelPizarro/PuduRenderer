#include <FileManager.h>
#include "CatDiorama.h"
#include "EntityManager.h"
#include "StringUtils.h"

namespace fs = std::filesystem;


bool showAxis;

void CatDiorama::OnRun()
{
    m_puduRenderer.Render(&m_scene);
    return;
    float speed = 0.15f;
    float phase = Time.Time() * speed;
    float radius = 15 - sin(phase)*10;
    float pich = 45;


    float x = cos(-phase) * radius;
    float z = sin(-phase) * radius;
    float y = sin(glm::radians(30.f - sin(phase)*20) ) * radius;

    vec3 pos = vec3(x, y, z);
    m_camera.Transform.SetLocalPosition(pos);

    vec3 forward = glm::normalize(vec3(0) - pos);

    m_camera.Transform.SetForward(forward, {0, 1, 0});
}

void CatDiorama::OnInit()
{
    m_camera = {};
    m_camera.Transform.SetLocalPosition({0, 0, -8});
    m_camera.Transform.SetLocalRotationEuler({8.,0,0});
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

    SamplerCreationData samplerCreationData{};
    samplerCreationData.wrap = true;

    TextureLoadSettings waterNormalTexSettings{};
    waterNormalTexSettings.bindless = false;
    waterNormalTexSettings.name = "WaterNormal";
    waterNormalTexSettings.format = VK_FORMAT_R8G8B8A8_UNORM;
    waterNormalTexSettings.samplerData = samplerCreationData;

    TextureLoadSettings skyTexSettings{};
    skyTexSettings.bindless = false;
    skyTexSettings.name = "Sky";
    skyTexSettings.format = VK_FORMAT_R8G8B8A8_SRGB;
    skyTexSettings.samplerData = samplerCreationData;
    skyTexSettings.textureType = TextureType::Texture_Cube;

    const auto skyTexture = Graphics.LoadTextureCube("textures/skyCube.ktx",skyTexSettings);

    m_cubemapTexture = Graphics.LoadTextureCube(cubeMapPath, cubemapSettings);
    const auto waternormalTex = Graphics.LoadTexture2D("textures/water_normal.png",waterNormalTexSettings);
    const auto transParentShader = Graphics.CreateShader("transparent.slang", "transparent");
    const auto waterShader = Graphics.CreateShader("water.shader.slang", "water");

    const auto sphereModel = FileManager::LoadGltfScene("models/sphere.gltf");
    const auto axisModel = FileManager::LoadGltfScene("models/axis.gltf");
    const auto overlayShader = Graphics.CreateShader("overlay.slang", "overlay");

    const auto catScene = FileManager::LoadGltfScene("models/Diorama_Cat/CatDiorama.gltf");

    const auto skyboxModel =std::dynamic_pointer_cast<RenderEntity>( FileManager::LoadGltfScene("models/skybox.gltf"));
    auto skyboxShader = Graphics.CreateShader("skybox.shader.slang", "skybox");

    auto planeMaterial = Graphics.Resources()->AllocateMaterial();
    planeMaterial->name ="PlaneMat";
    planeMaterial->SetShader(waterShader);
    planeMaterial->SetProperty("material.normalTex",waternormalTex);
    planeMaterial->SetProperty("material.skyTex",skyTexture);

    auto planeModel = Graphics.CreateModel(Graphics.GetDefaultQuad(), planeMaterial);

    auto planeName = std::string("Plane");
    auto planeEntity = EntityManager::AllocateRenderEntity(planeName, planeModel);
    planeEntity->GetTransform().SetLocalPosition({0, 0, 0});
    planeEntity->GetTransform().SetLocalScale({3, 3, 3});
    planeEntity->GetRenderSettings().layer = 1;



    // for (const auto& e : axisModel)
    // {
    //     RenderEntitySPtr re = std::dynamic_pointer_cast<RenderEntity>(e);
    //     re->GetTransform().SetLocalPosition({0, 0, 0});
    //     //re->GetTransform().SetLocalScale({ 3,3,3 });
    //     auto& [layer] = re->GetRenderSettings();
    //     layer = 2;
    //
    //     if (re != nullptr)
    //     {
    //         auto mat = re->GetModel()->Materials[0];
    //         mat->SetShader(overlayShader);
    //     }
    // }

    auto waterModel =catScene->GetChildByName<RenderEntity>("WATER");
    auto catModel =  catScene->GetChildByName<RenderEntity>("CAT");
    auto groundModel =catScene->GetChildByName<RenderEntity>("TERRAIN");


    catModel->GetModel()->Materials[0]->SetShader(standardShader);
    groundModel->GetModel()->Materials[0]->SetShader(standardShader);

    auto waterMaterial = waterModel->GetModel()->Materials[0];
    waterMaterial->SetShader(waterShader);
    waterMaterial->SetProperty("material.normalTex",waternormalTex);
    waterMaterial->SetProperty("material.skyTex",skyTexture);


    // for (const auto& e : sphereModel)
    // {
    //     RenderEntitySPtr re = std::dynamic_pointer_cast<RenderEntity>(e);
    //     re->GetTransform().SetLocalPosition({0, 5, 0});
    //     re->GetTransform().SetLocalScale({3, 3, 3});
    //     auto& [layer] = re->GetRenderSettings();
    //     layer = 1;
    //
    //     if (re != nullptr)
    //     {
    //         const auto mat = re->GetModel()->Materials[0];
    //         mat->SetShader(transParentShader);
    //     }
    // }

    skyboxModel->GetTransform().SetLocalPosition({0, 5, 0});
    skyboxModel->GetTransform().SetLocalScale({60, 60, 60});
    const auto mat = skyboxModel->GetModel()->Materials[0];
    mat->SetShader(skyboxShader);
    mat->SetProperty("material.skyboxTex", skyTexture);

   // m_scene.AddEntities(axisModel);
    m_scene.AddEntity(skyboxModel);
    m_scene.AddEntity(catScene);
 //   m_scene.AddEntities(sphereModel);
}

void CatDiorama::DrawImGUI()
{
    using namespace StringUtils;
    auto entities = m_scene.GetEntities();
    ImGui::Text(Format("FPS: {}", Time.GetFPS()).c_str());
    ImGui::Text(Format("Time: {}", Time.Time()).c_str());
    ImGui::Text(Format("DeltaTime: {}", Time.DeltaTime()).c_str());

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

    ImGui::Text("Camera");
    ImGuiUtils::DrawTransform(m_camera.Transform);

    ImGui::Text("Light");
    vec3 forward = directionalLight.GetTransform().GetForward();
   if ( ImGui::InputFloat3("Light Direction",&forward[0]))
   {
       directionalLight.GetTransform().SetForward(normalize(forward), {0,1,0});
   }
}

void CatDiorama::OnCleanup()
{

}

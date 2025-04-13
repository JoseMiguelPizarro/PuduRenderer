//
// Created by Administrator on 4/12/2025.
//

#include "Test_PBR.h"

#include "FileManager.h"

void Test_PBR::OnRun()
{
    m_puduRenderer.Render(&m_scene);
}

void Test_PBR::OnInit()
{
    m_camera = {};
    m_camera.Transform.SetLocalPosition({0, 0, -8});
    m_camera.Transform.SetForward({0, 0, 1}, {0, 1, 0});
    Projection projection;

    projection.Width = Graphics.WindowWidth;
    projection.Height = Graphics.WindowHeight;
    projection.Fov = 45;

    m_camera.Projection = projection;

    m_scene = Scene(&Time);
    m_scene.camera = &m_camera;
    TargetFPS = 30;

    m_puduRenderer.Init(&Graphics, this);

    standardShader = Graphics.CreateShader("standardSurface.shader.slang", "standard");

    projection.nearPlane = 5;
    projection.farPlane = 50;
    directionalLight = {};
    directionalLight.Projection = projection;
    directionalLight.GetTransform().SetForward({1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f});
    directionalLight.GetTransform().SetLocalPosition({20, 20, 20});
    m_scene.directionalLight = &directionalLight;


    auto sphere = FileManager::LoadGltfScene("models/sphere.gltf");
    std::dynamic_pointer_cast<RenderEntity>(sphere)->GetModel()->Materials[0]->SetShader(standardShader);

    m_scene.AddEntity(sphere);
}

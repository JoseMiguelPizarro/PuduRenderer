#include "TriangleApp.h"

#include <FileManager.h>

void TriangleApp::OnRun()
{
	Graphics.DrawFrame();
}

void TriangleApp::OnInit()
{
	auto modelData = FileManager::LoadModelObj(MODEL_PATH);

	m_camera = {};
	m_camera.Transform.SetForward(vec3(0, 0, -1), vec3(0, 1, 0));
	m_camera.Transform.Position = { 0, 0.0f, 7.0f };
	m_camera.Width = Graphics.WindowWidth;
	m_camera.Height = Graphics.WindowHeight;

	Graphics.SetCamera(&m_camera);

	m_scene = Scene{};

	m_modelMesh = Graphics.CreateMesh(modelData);
	
	m_texture = Graphics.CreateTexture(TEXTURE_PATH);
	Material material{};
	material.Texture = &m_texture;
	m_model = Graphics.CreateModel(&m_modelMesh, material);
	m_model.Transform.Rotation = vec3(0, -90, 0);
	m_scene.AddModel(&m_model);

	m_model2 = Graphics.CreateModel(&m_modelMesh, material);
	m_model2.Transform.Position = vec3(-4, 0, -5);
	m_model2.Transform.Rotation = vec3(0, -120, 0);

	m_model3 = Graphics.CreateModel(&m_modelMesh, material);
	m_model3.Transform.Position = vec3(4, 0, -8);
	m_model3.Transform.Rotation = vec3(0, -30, 0);

	m_scene.AddModel(&m_model2);
	m_scene.AddModel(&m_model3);

	Graphics.SceneToRender = m_scene;
}

void TriangleApp::OnCleanup()
{
	Graphics.DestroyMesh(&m_modelMesh);
	Graphics.DestroyTexture(m_texture);
}

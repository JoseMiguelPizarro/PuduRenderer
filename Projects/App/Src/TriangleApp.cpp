#include <filesystem>
#include <fmt/core.h>
#include <Logger.h>

#include <FileManager.h>
#include "TriangleApp.h"
#include "EntityManager.h"
#include "MeshManager.h"

namespace fs = std::filesystem;

void TriangleApp::OnRun()
{
	m_puduRenderer.sceneToRender = &m_scene;
	m_puduRenderer.Render();

	return;

	float radius = 20;
	float pich = 45;

	float speed = 0.35f;
	float phase = Time.Time() * speed;

	float x = cos(phase) * radius;
	float z = sin(phase) * radius;
	float y = sin(glm::radians(60.f)) * radius;

	vec3 pos = vec3(x, y, z);
	m_camera.Transform.LocalPosition = pos;

	vec3 forward = glm::normalize(vec3(0) - pos);

	m_camera.Transform.SetForward(forward, { 0,1,0 });
}

void TriangleApp::OnInit()
{
	m_camera = {};
	m_camera.Transform.SetForward(vec3(0, -.7, -1), vec3(0, 1, 0));
	m_camera.Transform.LocalPosition = { 0, 14.0f, 23.0f };
	m_camera.Width = Graphics.WindowWidth;
	m_camera.Height = Graphics.WindowHeight;
	m_camera.Fov = 45;

	m_scene = Scene(&Time);
	m_scene.camera = &m_camera;
	TargetFPS = 60;

	m_puduRenderer.Init(&Graphics, this);
	m_puduRenderer.LoadFrameGraph(FileManager::GetAssetPath(frameGraphPath));

	auto fragmentShaderPath = FileManager::GetAssetPath("Shaders/triangle.frag");
	auto vertexShaderPath = FileManager::GetAssetPath("Shaders/triangle.vert");
	standardShader = Graphics.CreateShader(fragmentShaderPath, vertexShaderPath, "standard");
	standardShader->name = "Standard";

	directionalLight = {};
	directionalLight.direction = { 1.0f,1.0f,1.0f };
	m_scene.directionalLight = &directionalLight;

	LoadGameboyModel();
}

void TriangleApp::DrawImGUI()
{
	auto entities = m_scene.GetEntities();
	ImGui::Text(fmt::format("Time: {}", Time.Time()).c_str());
	ImGui::Text(fmt::format("FPS: {}", Time.GetFPS()).c_str());
	ImGui::Text(fmt::format("DeltaTime: {}", Time.DeltaTime()).c_str());

	ImGuiUtils::DrawEntityTree(entities);
}

void TriangleApp::OnCleanup()
{
	auto renderEntities = m_scene.GetRenderEntities();
	for (auto entity : renderEntities)
	{
		auto model = entity->GetModel();
		for (auto mesh : model.Meshes) {
			mesh->Dispose();
		}
		for (auto material : model.Materials) {
			material.Texture->Dispose();
		}
	}
}

void TriangleApp::LoadGameboyModel()
{
	auto gltfScene = FileManager::LoadGltfScene(GameboyModelPath);

	for (auto e : gltfScene)
	{
		RenderEntitySPtr re = std::dynamic_pointer_cast<RenderEntity>(e);

		if (re != nullptr)
		{
			re->GetModel().Materials[0].Shader = standardShader;
		}
	}

	m_scene.AddEntities(gltfScene);
}

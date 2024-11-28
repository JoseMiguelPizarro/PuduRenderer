#include <filesystem>
#include <fmt/core.h>
#include <Logger.h>

#include <FileManager.h>
#include "TriangleApp.h"
#include "EntityManager.h"

namespace fs = std::filesystem;

void TriangleApp::OnRun()
{
	m_puduRenderer.sceneToRender = &m_scene;
	m_puduRenderer.Render();

	float radius = 20;
	float pich = 45;

	float speed = 0.35f;
	float phase = Time.Time() * speed;

	float x = cos(phase) * radius;
	float z = sin(phase) * radius;
	float y = sin(glm::radians(60.f)) * radius;

	vec3 pos = vec3(x, y, z);
	m_camera.Transform.SetLocalPosition(pos);

	vec3 forward = glm::normalize(vec3(0) - pos);

	m_camera.Transform.SetForward(forward, { 0,1,0 });
}

void TriangleApp::OnInit()
{
	m_camera = {};
	m_camera.Transform.SetForward(vec3(0, -.7, -1), vec3(0, 1, 0));
	m_camera.Transform.SetLocalPosition({ 0, 14.0f, 23.0f });
	Projection projection;

	projection.Width = Graphics.WindowWidth;
	projection.Height = Graphics.WindowHeight;
	projection.Fov = 45;

	m_camera.Projection = projection;

	m_scene = Scene(&Time);
	m_scene.camera = &m_camera;
	TargetFPS = 60;

	m_puduRenderer.Init(&Graphics, this);
	//m_puduRenderer.LoadFrameGraph(FileManager::GetAssetPath(frameGraphPath));

	auto fragmentShaderPath = FileManager::GetAssetPath("Shaders/triangle.frag");
	auto vertexShaderPath = FileManager::GetAssetPath("Shaders/triangle.vert");
	auto cubeShaderPath = FileManager::GetAssetPath("Shaders/cubemap.frag");

	standardShader = Graphics.CreateShader(fragmentShaderPath, vertexShaderPath, "standard");
	cubemapShader = Graphics.CreateShader(cubeShaderPath, vertexShaderPath, "Cubemap");

	projection.nearPlane = 5;
	projection.farPlane = 50;
	directionalLight = {};
	directionalLight.Projection = projection;
	directionalLight.GetTransform().SetForward({ -1.0f, -1.0f,-1.0f }, { 0.0f,1.0f,0.0f });
	directionalLight.GetTransform().SetLocalPosition({ 20,20,20 });
	m_scene.directionalLight = &directionalLight;

	LoadGameboyModel();

	TextureCreationSettings planeSettings{};
	planeSettings.bindless = false;
	planeSettings.name = "Plane";
	m_planeTexture = Graphics.LoadTexture2D(planeTexturePath, planeSettings);

	TextureCreationSettings cubemapSettings{};
	cubemapSettings.bindless = false;
	cubemapSettings.name = "Cubemap";
	cubemapSettings.format = VK_FORMAT_R8G8B8A8_UNORM;

	m_cubemapTexture = Graphics.LoadTextureCube(cubeMapPath, cubemapSettings);

	auto cube = FileManager::LoadGltfScene("models/sphere.gltf");

	for (auto e : cube) {
		RenderEntitySPtr re = std::dynamic_pointer_cast<RenderEntity>(e);
		re->GetTransform().SetLocalPosition({ 0,5,0 });
		re->GetTransform().SetLocalScale({ 3,3,3 });

		if (re != nullptr)
		{
			auto mat = &re->GetModel().Materials[0];
			mat->Shader = cubemapShader;
			mat->SetProperty("testText", m_cubemapTexture);
		}
	}

	m_scene.AddEntities(cube);
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
			Graphics.DestroyMesh(mesh);
		}
		for (auto material : model.Materials) {
			if (material.Texture != nullptr)
			{
				Graphics.DestroyTexture(material.Texture);
			}
		}
	}
}

void TriangleApp::LoadGameboyModel()
{
	auto gltfScene = FileManager::LoadGltfScene(GameboyModelPath);

	for (auto& e : gltfScene)
	{
		RenderEntitySPtr re = std::dynamic_pointer_cast<RenderEntity>(e);

		if (re != nullptr)
		{
			re->GetModel().Materials[0].Shader = standardShader;
		}
	}

	m_scene.AddEntities(gltfScene);
}

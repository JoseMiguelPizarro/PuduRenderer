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
	Graphics.DrawFrame();
}

void TriangleApp::OnInit()
{
	m_camera = {};
	m_camera.Transform.SetForward(vec3(0, 0, -1), vec3(0, 1, 0));
	m_camera.Transform.LocalPosition = { 0, 0.0f, 7.0f };
	m_camera.Width = Graphics.WindowWidth;
	m_camera.Height = Graphics.WindowHeight;

	m_scene = Scene(&Time);
	m_scene.camera = &m_camera;

	//m_planeTexture = Graphics.GetResources().AllocateTexture(planeTexturePath);
	//auto planeModelData = FileManager::LoadModelObj(planeModelPath.string());

	//m_planeMesh = MeshManager::AllocateMesh(planeModelData);
	//Material planeMaterial{};
	//planeMaterial.Texture = m_planeTexture;
	//m_planeModel = Graphics.CreateModel(m_planeMesh, planeMaterial);

	//auto name = std::string("Plane");
	//auto planeEntity = EntityManager::AllocateRenderEntity(name, m_planeModel);
	//m_scene.AddEntity(planeEntity);

	LoadGameboyModel();
	Graphics.SceneToRender = &m_scene;
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

	m_scene.AddEntities(gltfScene);
}

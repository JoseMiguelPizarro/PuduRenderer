#include <filesystem>
#include <fmt/core.h>
#include <Logger.h>

#include "TriangleApp.h"
#include "RenderEntityManager.h"
#include <FileManager.h>


namespace fs = std::filesystem;

void TriangleApp::OnRun()
{
	float time = Time.Time() * 0.1f;
	vec3 centroid = (m_model.Transform.LocalPosition + m_model2.Transform.LocalPosition + m_model3.Transform.LocalPosition) / 3.0f;

	float radius = 15;
	float x = cos(time) * radius;
	float y = sin(time) * radius;

	vec3 pos = centroid + vec3(x, m_camera.Transform.LocalPosition.y, y);
	vec3 dir = -normalize(pos - centroid);

	vec3 r = cross({ 0,1,0 }, dir);
	vec3 u = cross(dir, r);

	/*m_camera.Transform.Position = pos;
	m_camera.Transform.SetForward(dir, u);*/

	Graphics.DrawFrame();
}


void TriangleApp::OnInit()
{
	auto modelData = FileManager::LoadModelObj(MODEL_PATH.string());

	m_camera = {};
	m_camera.Transform.SetForward(vec3(0, 0, -1), vec3(0, 1, 0));
	m_camera.Transform.LocalPosition = { 0, 0.0f, 7.0f };
	m_camera.Width = Graphics.WindowWidth;
	m_camera.Height = Graphics.WindowHeight;

	m_scene = Scene(&Time);
	m_scene.Camera = &m_camera;

	m_modelMesh = Graphics.CreateMesh(modelData);

	m_texture = Graphics.CreateTexture(TEXTURE_PATH);
	Material material{};
	material.Texture = m_texture;
	m_model = Graphics.CreateModel(m_modelMesh, material);
	m_model.Transform.LocalRotation = vec3(0, -90, 0);

	m_model2 = Graphics.CreateModel(m_modelMesh, material);
	m_model2.Transform.LocalPosition = vec3(-4, 0, -5);
	m_model2.Transform.LocalRotation = vec3(0, -120, 0);

	m_model3 = Graphics.CreateModel(m_modelMesh, material);
	m_model3.Transform.LocalPosition = vec3(4, 0, -8);
	m_model3.Transform.LocalRotation = vec3(0, -30, 0);

	m_planeTexture = Graphics.CreateTexture(planeTexturePath);
	auto planeModelData = FileManager::LoadModelObj(planeModelPath.string());
	m_planeMesh = Graphics.CreateMesh(planeModelData);
	Material planeMaterial{};
	planeMaterial.Texture = m_planeTexture;
	m_planeModel = Graphics.CreateModel(m_planeMesh, planeMaterial);

	//m_scene.AddModel(m_model);
	//m_scene.AddModel(m_model2);
	//m_scene.AddModel(m_model3);
	//m_scene.AddModel(m_planeModel);

	LoadGameboyModel();
	Graphics.SceneToRender = &m_scene;
}

void TriangleApp::OnCleanup()
{
	Graphics.DestroyMesh(&m_modelMesh);
	Graphics.DestroyTexture(m_texture);
	Graphics.DestroyTexture(m_planeTexture);
	Graphics.DestroyMesh(&m_planeMesh);
}

void TriangleApp::LoadGameboyModel()
{
	auto data = FileManager::LoadModelGltf(GameboyModelPath);

	std::vector<Model> gameboyModels;
	for (auto d : data) {
		Model m = Graphics.CreateModel(d);
		gameboyModels.push_back(m);

		auto re = RenderEntityManager::AllocateEntity(m);
		GameboyEntities.push_back(re);
		m_scene.AddRendererEntity(re);
	}
}

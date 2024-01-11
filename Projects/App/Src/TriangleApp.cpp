#include "TriangleApp.h"

#include <FileManager.h>

void TriangleApp::OnRun()
{
	float time = Time.Time() * 0.1f;
	vec3 centroid = (m_model.Transform.Position + m_model2.Transform.Position + m_model3.Transform.Position) / 3.0f;

	float radius = 15;
	float x = cos(time) * radius;
	float y = sin(time) * radius;

	vec3 pos = centroid + vec3(x, m_camera.Transform.Position.y, y);
	vec3 dir = -normalize(pos - centroid);

	vec3 r = cross({ 0,1,0 }, dir);
	vec3 u = cross(dir, r);

	m_camera.Transform.Position = pos;
	m_camera.Transform.SetForward(dir, u);

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

	m_scene = Scene(&Time);
	m_scene.Camera = &m_camera;

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


	m_planeTexture = Graphics.CreateTexture(planeTexturePath);
	auto planeModelData = FileManager::LoadModelObj(planeModelPath);
	m_planeMesh = Graphics.CreateMesh(planeModelData);
	Material planeMaterial{};
	planeMaterial.Texture = &m_planeTexture;
	m_planeModel = Graphics.CreateModel(&m_planeMesh, planeMaterial);

	m_scene.AddModel(&m_model2);
	m_scene.AddModel(&m_model3);
	m_scene.AddModel(&m_planeModel);

	Graphics.SceneToRender = m_scene;
}

void TriangleApp::OnCleanup()
{
	Graphics.DestroyMesh(&m_modelMesh);
	Graphics.DestroyTexture(m_texture);
	Graphics.DestroyTexture(m_planeTexture);
	Graphics.DestroyMesh(&m_planeMesh);
}

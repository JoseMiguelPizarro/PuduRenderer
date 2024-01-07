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

	m_modelMesh = Graphics.CreateMesh(modelData);
	m_texture = Graphics.CreateTexture(TEXTURE_PATH);
	m_drawCall = DrawCall(&m_modelMesh, &m_texture);
	Graphics.m_drawCall = m_drawCall;
}

void TriangleApp::OnCleanup()
{
	Graphics.DestroyMesh(&m_modelMesh);
	Graphics.DestroyTexture(m_texture);
}

#include "TriangleApp.h"

#include <FileManager.h>

void TriangleApp::OnRun() 
{
	Graphics.DrawFrame();
}

void TriangleApp::OnInit()
{
	auto modelData = FileManager::LoadModelObj(MODEL_PATH);
	m_modelMesh = Graphics.CreateMesh(modelData);
	m_drawCall = DrawCall(&m_modelMesh);
	Graphics.m_drawCall = m_drawCall;
}

void TriangleApp::OnCleanup()
{
	Graphics.DestroyMesh(&m_modelMesh);
}

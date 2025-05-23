#pragma once
#include <PuduCore.h>
#include <PuduRenderer.h>
#include <FrameGraph/FrameGraph.h>
#include <PuduApp.h>
#include "Mesh.h"
#include "Scene.h"
#include "Camera.h"
#include "Model.h"
#include "Texture2d.h"
#include "ImGuiUtils.h"
#include <memory.h>
#include <memory_resource>
#include <filesystem>

using namespace Pudu;

class CatDiorama : public PuduApp
{
public:
	void OnRun() override;
	void OnInit() override;

	virtual void DrawImGUI() override;

private:
	void OnCleanup() override;

	std::filesystem::path frameGraphPath = "FrameGraphs/Standard.sg";

	std::filesystem::path planeTexturePath = "models/floor/Grass.png";
	std::filesystem::path planeModelPath = "models/floor/floor.obj";
	fs::path cubeMapPath = "textures/cubemap_yokohama_rgba.ktx";


	Scene m_scene;
	Camera m_camera;
	SPtr<Mesh> m_planeMesh;
	Model m_planeModel;
	SPtr<Texture2d> m_planeTexture;
	SPtr<TextureCube> m_cubemapTexture;
	PuduRenderer m_puduRenderer;
	SPtr<Shader> standardShader;
	SPtr<Shader> cubemapShader;
	Light directionalLight;
};

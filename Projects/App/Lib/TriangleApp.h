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

using namespace Pudu;

class TriangleApp : public PuduApp
{
public:
	void OnRun() override;
	void OnInit() override;

	virtual void DrawImGUI() override;

private:
	void OnCleanup() override;

	void LoadGameboyModel();

	std::filesystem::path GameboyModelPath = "models/gameboy/gameboy.gltf";

	std::filesystem::path frameGraphPath = "FrameGraphs/Standard.sg";

	std::filesystem::path planeTexturePath = "models/floor/Grass.png";
	std::filesystem::path planeModelPath = "models/floor/floor.obj";

	std::vector<RenderEntitySPtr> GameboyEntities;

	Scene m_scene;
	Camera m_camera;
	SPtr<Mesh> m_planeMesh;
	Model m_planeModel;
	SPtr<Texture2d> m_planeTexture;
	PuduRenderer m_puduRenderer;
	SPtr<Shader> standardShader;
};

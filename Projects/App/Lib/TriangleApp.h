#pragma once
#include <PuduApp.h>
#include "Mesh.h"
#include "Scene.h"
#include "Camera.h"
#include "Model.h"
#include "Texture2d.h"

using namespace Pudu;

class TriangleApp : public PuduApp
{
public:
    void OnRun() override;
    void OnInit() override;

private:
    void OnCleanup() override;

    void LoadGameboyModel();

    std::string TEXTURE_PATH = "models/chocobo/chocobo.png";
    std::string MODEL_PATH = "models/chocobo/chocobo.obj";
    std::string GameboyModelPath = "models/gameboy/scene.gltf";

    std::string planeTexturePath = "models/floor/Grass.png";
    std::string planeModelPath= "models/floor/floor.obj";

    Scene m_scene;
    Camera m_camera;
    Mesh m_modelMesh;
    Mesh m_planeMesh;
    Model m_model;
    Model m_model2;
    Model m_model3;
    Model m_planeModel;
    Texture2d m_texture;
    Texture2d m_planeTexture;
};

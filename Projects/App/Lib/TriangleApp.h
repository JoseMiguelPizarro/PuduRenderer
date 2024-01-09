#pragma once
#include <PuduApp.h>

#include "Mesh.h"

using namespace Pudu;

class TriangleApp : public PuduApp
{
public:
    void OnRun() override;
    void OnInit() override;

private:
    void OnCleanup() override;

    std::string TEXTURE_PATH = "models/chocobo/chocobo.png";
    std::string MODEL_PATH = "models/chocobo/chocobo.obj";
    Scene m_scene;
    Camera m_camera;
    Mesh m_modelMesh;
    Model m_model;
    Model m_model2;
    Model m_model3;
    Texture2d m_texture;
};

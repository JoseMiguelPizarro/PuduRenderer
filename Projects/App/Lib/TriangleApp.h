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
    DrawCall m_drawCall;
    Camera m_camera;
    Mesh m_modelMesh;
    Texture2d m_texture;
};

//
// Created by Jose on 4/12/2025.
//

#pragma once

#include "OverlayQuadEntity.h"
#include "PuduCore.h"
#include "PuduApp.h"
#include "PuduRenderer.h"

using namespace Pudu;

class Test_PBR:public PuduApp {
    void OnRun() override;
    void OnInit() override;

public:
    void DrawImGUI() override;

private:
    Camera m_camera;
    Scene m_scene;
    PuduRenderer m_puduRenderer;
    SPtr<Shader> standardShader;
    Light directionalLight;
    SPtr<OverlayQuadTextureArrayEntity> m_arrayQO;
};



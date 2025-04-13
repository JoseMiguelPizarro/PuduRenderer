//
// Created by Jose on 4/12/2025.
//

#pragma once

#include "PuduCore.h"
#include "PuduApp.h"
#include "PuduRenderer.h"

using namespace Pudu;

class Test_PBR:public PuduApp {
    void OnRun() override;
    void OnInit() override;

private:
    Camera m_camera;
    Scene m_scene;
    PuduRenderer m_puduRenderer;
    SPtr<Shader> standardShader;
    Light directionalLight;
};



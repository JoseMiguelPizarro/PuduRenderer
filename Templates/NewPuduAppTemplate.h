//
// Created by Hojaverde on 8/23/2025.
//

#pragma once

#include "PuduApp.h"
#include "PuduRenderer.h"

using namespace Pudu;

class NewPuduAppTemplate:public PuduApp {
    void OnRun() override;
    void OnInit() override;

public:
    void DrawImGUI() override;
private:
    Camera m_camera;
    Scene m_scene;
    PuduRenderer m_puduRenderer;
    Light m_directionalLight;
    float m_camRadius = 10;
    float m_lightDistance = 10;
};



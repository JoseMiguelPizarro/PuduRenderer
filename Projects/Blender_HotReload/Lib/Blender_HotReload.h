//
// Created by Hojaverde on 8/23/2025.
//

#pragma once

#include "PuduApp.h"
#include "PuduRenderer.h"

using namespace Pudu;
namespace Pudu
{
    class Blender_HotReload:public PuduApp {
        void OnRun() override;
        void OnInit() override;

    public:
        void DrawImGUI() override;
    private:
        Camera m_camera;
        Scene m_scene;
        PuduRenderer m_puduRenderer;
        float m_camRadius = 10;
        float m_lightDistance = 10;
        EntitySPtr m_model;
    };
}





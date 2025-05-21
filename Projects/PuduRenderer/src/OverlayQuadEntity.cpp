//
// Created by Hojaverde on 5/21/2025.
//

#include "OverlayQuadEntity.h"

#include "PuduGraphics.h"

namespace Pudu
{
    OverlayQuadEntity::OverlayQuadEntity(PuduGraphics* gfx)
    {
        auto mesh = gfx->GetDefaultQuad();
        auto shader = gfx->GetDefaultOverlayShader();

        auto material = gfx->Resources()->AllocateMaterial();
        material->SetShader(shader);

        Model m;
        m.Name = "OverlayQuad";
        m.Materials.push_back(material);
        m.Meshes.push_back(mesh);

        auto model = gfx->CreateModel(mesh, material);
        SetModel(model);

        m_renderSettings.layer = 2;
    }

    SPtr<Material> OverlayQuadEntity::GetMaterial()
    {
        return m_model.Materials[0];
    }

    void OverlayQuadEntity::SetPtr(SPtr<RenderEntity> ptr)
    {
        m_entitySPtr = ptr;
    }

    void OverlayQuadEntity::SetPositionAndSize(float x, float y, float width, float height)
    {
        auto m = m_model.Materials[0];

        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;

        m->SetProperty("overlayData.x",x);
        m->SetProperty("overlayData.y", y);
        m->SetProperty("overlayData.width", width);
        m->SetProperty("overlayData.height", height);
    }
} // Pudu

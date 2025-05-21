//
// Created by Hojaverde on 5/21/2025.
//

#pragma once
#include "RenderEntity.h"

namespace Pudu {

class PuduGraphics;

class OverlayQuadEntity : public RenderEntity {

public:
    explicit OverlayQuadEntity(PuduGraphics* gfx);
    SPtr<Material> GetMaterial();

    void SetPtr(SPtr<RenderEntity> ptr);
    ///Values in [0,1] Screen coordinates
    void SetPositionAndSize(float x, float y, float width, float height);
private:
    float width, height, x,y;

};

} // Pudu


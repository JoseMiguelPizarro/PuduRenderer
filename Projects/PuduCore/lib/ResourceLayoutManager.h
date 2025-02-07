//
// Created by Hojaverde on 2/1/2025.
//

#pragma once

#include <unordered_map>

#include "Texture.h"
#include "Enums/ResourceUsage.h"

namespace Pudu
{
    class ResourceLayoutManager
    {
        public:
        void SetTextureUsage(Texture texture, ResourceUsage usage);


    private:
        std::unordered_map<Texture, ResourceUsage> m_textures;
    };
} // Pudu

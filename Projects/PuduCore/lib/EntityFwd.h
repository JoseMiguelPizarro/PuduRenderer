//
// Created by Hojaverde on 7/11/2025.
//

#pragma once
#include "Entity.h"
#include "Resources/ResourceHandle.h"

namespace Pudu
{
    class Entity;
    class RenderEntity;
    typedef std::shared_ptr<Entity> EntitySPtr;
    typedef std::shared_ptr<RenderEntity> RenderEntitySPtr;
    typedef ResourceHandle<Entity> EntityHandle;
    typedef ResourceHandle<RenderEntity> RenderEntityHandle;


}

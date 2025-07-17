//
// Created by Hojaverde on 7/10/2025.
//

#pragma once
#include "EntityFwd.h"

namespace Pudu
{
    class EntityPool<T>
    {
        public:
        EntitySPtr GetEntity(EntityHandle handle);
        void ReleaseEntity(EntityHandle handle);
        EntityHandle AllocateEntity();
    };
}

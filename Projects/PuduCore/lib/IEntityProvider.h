//
// Created by Hojaverde on 7/10/2025.
//

#pragma once
#include "EntityFwd.h"
#include "Logger.h"
#include "Resources/IResourceProvider.h"

namespace Pudu
{
    class IEntityProvider : public IResourceProvider<EntitySPtr>
    {
        EntitySPtr GetResource(Handle id) override { ASSERT(false, "EntityProvider GetResource not implemented");return nullptr; };
    };
}

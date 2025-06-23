//
// Created by Administrator on 6/23/2025.
//
#pragma once
#include "PuduCore.h"
#include "ResourceType.h"

namespace Pudu
{
    class IGPUResourceProvider
    {
    public:
        virtual ~IGPUResourceProvider() = default;
        virtual void* GetResource(u32 id, GPUResourceType type){throw std::exception("Not implemented");};
    };
}

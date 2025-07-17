//
// Created by Administrator on 7/11/2025.
//

#pragma once
#include "PuduCore.h"

namespace Pudu
{
    template<typename T>
    class IResourceProvider
    {
        public:
        virtual ~IResourceProvider() = default;

        virtual UPtr<T> GetResource(Handle id) = 0;
    };

}

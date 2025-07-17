//
// Created by Administrator on 7/11/2025.
//

#pragma once
#include "PuduCore.h"
#include "IResourceProvider.h"

namespace Pudu
{
    template <typename T>
    struct ResourceHandle
    {
        ResourceHandle() = default;

        ResourceHandle(Handle handle, IResourceProvider<T>* provider) : handle(handle), m_provider(provider)
        {
        }

        ResourceHandle(const ResourceHandle<T>& other) : handle(other.handle), m_provider(other.m_provider)
        {
        }

        UPtr<T> Get()
        {
            return m_provider->GetResource(handle);
        }

        bool IsValid() const { return handle != K_INVALID_HANDLE; }

        Handle GetHandle() const { return handle; }

        bool operator==(const ResourceHandle<T>& other) const
        {
            return handle == other.handle;
        }

        bool operator!=(const ResourceHandle<T>& other) const
        {
            return handle != other.handle;
        }

    protected:
        Handle handle = K_INVALID_HANDLE;
        IResourceProvider<T>* m_provider;
    };
}

//
// Created by Hojaverde on 7/11/2025.
//

#pragma once
#include "IResourceProvider.h"
#include "PuduCore.h"
#include "PuduConstants.h"
#include "ResourceHandle.h"
#include "ResourcesPool.h"


namespace Pudu
{
    template <typename T>
    class Resource
    {
    public:
        virtual ~Resource() = default;
        ResourceHandle<T> GetHandle() const { return m_handle; };
        void SetHandle(ResourceHandle<T> handle) { m_handle = handle; };

        T Get() { return m_provider->GetResource(m_handle); }
        bool IsValid() const { return m_handle != K_INVALID_HANDLE; };

    protected:
        friend class ResourcePool<T>;
        ResourceHandle<T> m_handle;
        IResourceProvider<T>* m_provider;
    };
}

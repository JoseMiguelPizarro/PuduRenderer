#pragma once
#include <concepts>

namespace Pudu
{
    class PuduGraphics;

    class GPUResourceType
    {
    public:
        enum Type
        {
            Texture,
            Buffer,
            RenderPass,
            UNINITIALIZED
        };
    };

    struct GPUResourceHandleBase
    {
        uint32_t Index() const { return m_Index; }

        friend class GPUResourcesManager;
        void SetIndex(uint32_t index) { m_Index = index; }
        uint32_t m_Index;
    };
    
    class GPUResourceBase
    {
    };


    template <typename T>
        requires(std::convertible_to<T, GPUResourceBase>)
    struct GPUResourceHandle : public GPUResourceHandleBase
    {
    public:
        bool IsEqual(const GPUResourceHandle& other)
        {
            return this->m_Index == other.m_Index;
        }
    };


    template <typename T>
    class GPUResource : public GPUResourceBase
    {
    public:
        GPUResourceHandle<T> Handle() { return m_handle; }
        bool IsAllocated() { return m_allocated; }
        std::string name;


        virtual void Create(PuduGraphics* gpu)
        {
            if (m_allocated)
            {
                return;
            }

            OnCreate(gpu);
            m_allocated = true;
        }

        virtual GPUResourceType::Type Type() { return GPUResourceType::UNINITIALIZED; };
        virtual ~GPUResource() = default;

    protected:
        friend class GPUResourcesManager;
        GPUResourceHandle<T> m_handle;
        bool m_allocated;

        virtual void OnCreate(PuduGraphics* gpu)
        {
        };
    };
}

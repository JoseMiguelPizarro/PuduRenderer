#include "EntityManager.h"

namespace Pudu
{
    EntityHandle EntityManager::AllocateEntity()
    {
        auto e = std::make_shared<Entity>();

        e->m_handle = EntityHandle(m_entities.size(),this);
        m_entities.push_back(e);

        return e->m_handle;
    }

    EntityHandle EntityManager::AllocateEntity(std::string& name)
    {
        auto e = AllocateEntity();
        e.Get()->SetName(name);

        return e;
    }

    EntitySPtr EntityManager::GetEntity(Handle handle)
    {
        return GetResource(handle);
    }

    EntityHandle EntityManager::AllocateRenderEntity()
    {
        auto e = std::make_shared<RenderEntity>();
        e->m_handle = RenderEntityHandle(m_entities.size(), this);
        m_entities.push_back(e);

        return e->GetHandle();
    }

    EntityHandle EntityManager::AllocateRenderEntity(const char* name)
    {
        auto e = AllocateRenderEntity();
        e.Get()->SetName(name);

        return e;
    }

    EntityHandle EntityManager::AllocateRenderEntity(Model model)
    {
        auto e = AllocateRenderEntity();
        e.Get()->SetModel(model);

        return e;
    }

    RenderEntitySPtr EntityManager::AllocateRenderEntity(const char* name, Model model)
    {
        auto e = AllocateRenderEntity(name);
        e->SetModel(model);

        return e;
    }

    void EntityManager::DestroyEntity(RenderEntitySPtr entity)
    {
    }

    EntitySPtr EntityManager::GetResource(Handle id)
    {
        return m_entities[id];
    }
}

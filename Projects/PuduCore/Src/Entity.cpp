#include "Entity.h"

#include "Logger.h"

namespace Pudu
{
    Entity::Entity(std::string& name) : m_name(name)
    {
    }


    void Entity::SetName(std::string const name)
    {
        m_name = name;
    }

    const std::string& Entity::GetName()
    {
        return m_name;
    }

    Transform& Entity::GetTransform()
    {
        return m_transform;
    }

    void Entity::SetTransform(const Transform& t)
    {
        m_transform = t;
    }

    void Entity::SetParent(EntityHandle parent)
    {
        m_parent = parent;
        m_transform.SetParent(&parent.Get()->m_transform);

        parent.Get()->AddChild(m_handle);
    }

    EntitySPtr Entity::GetParent()
    {
        return m_parent.Get();
    }

    std::vector<EntityHandle> Entity::GetChildren()
    {
        return m_children;
    }

    EntitySPtr Entity::GetChildByName(const char* name)
    {
        for (auto& child : m_children)
        {
            const char* childName = child.Get()->GetName().c_str();
            if (strcmp(childName, name) == 0)
                return child.Get();
        }

        ASSERT(false, "Child not found");
        return nullptr;
    }


    size_t Entity::ChildCount()
    {
        return m_children.size();
    }

    void Entity::AttatchToScene(Scene& scene)
    {
    }

    EntitySPtr Entity::GetRoot() const
    {
        return GetRoot(m_handle);
    }

    EntitySPtr Entity::GetRoot(EntityHandle entity) const
    {
        if (!entity.Get()->m_parent.IsValid())
        {
            return entity.Get();
        }

        return GetRoot(entity.Get()->m_parent);
    }

    void Entity::AddChild(EntityHandle entity)
    {
        m_children.push_back(entity);
    }
}

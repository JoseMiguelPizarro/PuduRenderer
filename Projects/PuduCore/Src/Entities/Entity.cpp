#include "Entities/Entity.h"

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

    void Entity::SetTransform(Transform t)
    {
        m_transform = t;
    }

    void Entity::SetParent(EntitySPtr parent)
    {
        m_parent = parent;
        if (m_parent != nullptr)
        {
            m_transform.SetParent(&parent->m_transform);
            parent->AddChild(m_entitySPtr);
        }
    }

    EntitySPtr Entity::GetParent()
    {
        return m_parent;
    }

    std::vector<EntitySPtr> Entity::GetChildren()
    {
        return m_children;
    }

    EntitySPtr Entity::GetChildByName(const char* name)
    {
        for (auto& child : m_children)
        {
            const char* childName = child->GetName().c_str();
            if (strcmp(childName, name) == 0)
                return child;
        }

        ASSERT(false, "Child not found");
        return nullptr;
    }


    size_t Entity::ChildCount()
    {
        return m_children.size();
    }

    void Entity::RemoveChild(EntitySPtr child)
    {
        m_children.erase(std::ranges::remove(m_children, child).begin(), m_children.end());
        child->SetParent(nullptr);
    }

    void Entity::AttatchToScene(Scene& scene)
    {
    }

    EntitySPtr Entity::GetRoot() const
    {
        return GetRoot(m_entitySPtr);
    }

    EntitySPtr Entity::GetRoot(EntitySPtr const& entity) const
    {
        if (entity->m_parent == nullptr)
        {
            return entity;
        }

        return GetRoot(entity->m_parent);
    }

    void Entity::AddChild(EntitySPtr& entity)
    {
        m_children.push_back(entity);
    }
}

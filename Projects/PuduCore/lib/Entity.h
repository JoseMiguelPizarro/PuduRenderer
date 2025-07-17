#pragma once
#include <memory>
#include <Transform.h>
#include <vector>
#include <string>
#include "Concepts.h"
#include "PuduCore.h"
#include "EntityHandle.h"
#include "Resources/Resource.h"
#include "EntityFwd.h"

namespace Pudu
{
    class Scene;
    class EntityManager;

    class Entity : public Resource<Entity>
    {
    public:
        virtual std::string ClassName()
        {
            return "Entity";
        }

        void SetName(std::string const name);
        const std::string& GetName();

        Entity(): m_parent(), m_handle()
        {
        }
        ;
        Entity(std::string& name);

        Transform& GetTransform();
        void SetTransform(const Transform& t);
        void SetParent(EntityHandle parent);
        EntitySPtr GetParent();
        EntitySPtr GetRoot() const;
        std::vector<EntityHandle> GetChildren();

        EntitySPtr GetChildByName(const char* name);

        template <Derived<Entity> T>
        SPtr<T> GetChildByName(std::string const& name);

        size_t ChildCount();
        virtual void AttatchToScene(Scene& scene);

    protected:
        friend class EntityManager;

        UPtr<EntityManager> m_entityManager = nullptr;
        EntityHandle m_parent;
        EntityHandle m_handle;

        Transform m_transform;
        std::vector<EntityHandle> m_children;
        std::string m_name;
        EntitySPtr GetRoot(EntityHandle entity) const;
        void AddChild(EntityHandle entity);
    };

    template <Derived<Entity> T>
    SPtr<T> Entity::GetChildByName(std::string const& name)
    {
        return std::dynamic_pointer_cast<T>(GetChildByName(name.c_str()));
    }
}

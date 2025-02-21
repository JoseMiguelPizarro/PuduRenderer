#pragma once
#include <memory>
#include <Transform.h>
#include <vector>
#include <string>
#include "Concepts.h"
#include "PuduCore.h"

namespace Pudu {
	class Scene;
	class Entity
	{
		typedef std::shared_ptr<Entity> EntitySPtr;
	public:
		virtual std::string ClassName() {
			return "Entity";
		}

		void SetName(std::string const name);
		const std::string& GetName();
		Entity() {};
		Entity(std::string& name);

		Transform& GetTransform();
		void SetTransform(Transform t);
		void SetParent(EntitySPtr parent);
		EntitySPtr GetParent();
		EntitySPtr GetRoot() const;
		std::vector<EntitySPtr> GetChildren();

		EntitySPtr GetChildByName(std::string_view const& name);

		template <Derived<Entity> T>
		SPtr<T> GetChildByName(std::string const& name);

		size_t ChildCount();
		virtual void AttatchToScene(Scene& scene);

	protected:
		friend class EntityManager;

		EntitySPtr m_entitySPtr = nullptr;
		EntitySPtr m_parent = nullptr;

		Transform m_transform;
		std::vector<EntitySPtr> m_children;
		std::string m_name;
		EntitySPtr GetRoot(EntitySPtr const & entity) const;
		void AddChild(EntitySPtr& entity);

	};

	template <Derived<Entity> T>
	SPtr<T> Entity::GetChildByName(std::string const& name)
	{
		return std::dynamic_pointer_cast<T>(GetChildByName(name));
	}

	typedef std::shared_ptr<Entity> EntitySPtr;
}


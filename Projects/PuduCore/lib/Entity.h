#pragma once
#include <memory>
#include <Transform.h>
#include <vector>
#include <string>

namespace Pudu {
	class Scene;
	class Entity
	{
		typedef std::shared_ptr<Entity> EntitySPtr;
	public:
		void SetName(std::string& name);
		const std::string& GetName(std::string& name);

		Transform& GetTransform();
		void AddChild(EntitySPtr child);
		void SetParent(EntitySPtr parent);
		virtual void AttatchToScene(Scene& scene);

	protected:
		EntitySPtr m_entitySPtr;
		EntitySPtr m_parent;
		Transform m_transform;
		std::vector<EntitySPtr> m_children;
		std::string m_name;
	};

	typedef std::shared_ptr<Entity> EntitySPtr;
}


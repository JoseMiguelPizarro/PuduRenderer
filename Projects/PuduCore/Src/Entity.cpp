#include "Entity.h"

namespace Pudu {

	void Entity::SetName(std::string& name)
	{
		m_name = name;
	}

	const std::string& Entity::GetName(std::string& name)
	{
		return m_name;
	}

	Transform& Entity::GetTransform()
	{
		return m_transform;
	}
	void Entity::AddChild(EntitySPtr child)
	{
		m_children.push_back(child);

		auto g = child.get();
		child.get()->SetParent(m_entitySPtr);
	}
	void Entity::SetParent(EntitySPtr parent)
	{
		m_parent = parent;

		m_transform.ParentMatrix = parent.get()->GetTransform().GetTransformationMatrix();
	}
	void Entity::AttatchToScene(Scene& scene)
	{
	}
}
#include "Transform.h"
#include <glm/ext/matrix_transform.hpp>
#include "PuduMath.h"

using namespace glm;

namespace Pudu {
	void Transform::SetRotation(quat r)
	{
		LocalRotation = degrees(eulerAngles(r));
	}

	quat Transform::GetRotationQuat()
	{
		return quat(radians(LocalRotation));
	}

	mat4 Transform::GetTransformationMatrix()
	{
		mat4 rot = toMat4(GetRotationQuat());
		return ParentMatrix * translate(mat4(1.0f), LocalPosition) * scale(mat4(1.0f), LocalScale) * rot;
	}

	void Transform::SetForward(vec3 forward, vec3 up)
	{
		quat rot = PuduMath::LookRotation(normalize(forward), normalize(up));

		//vec3 rotEuler = degrees(PuduMath::EulerAnglesFromQuat(rot));
		vec3 rotEuler = degrees(eulerAngles(rot));
		LocalRotation = rotEuler;
	}

	vec3 Transform::GetForward()
	{
		return normalize(rotate(GetRotationQuat(), vec3(0, 0, 1)));
	}
	void Transform::UpdateWorldTransformRecursivelly()
	{
		if (m_parentTransform != nullptr)
		{
			ParentMatrix = m_parentTransform->GetTransformationMatrix();
		}
		else
		{
			ParentMatrix = mat4(1.0f);
		}

		for (auto child : Children) {
			child->UpdateWorldTransformRecursivelly();
		}
	}
	void Transform::SetParent(Transform* parent)
	{
		m_parentTransform = parent;
		parent->AddChild(this);
	}
	void Transform::AddChild(Transform* child)
	{
		Children.push_back(child);
		UpdateWorldTransformRecursivelly();
	}

	Transform* Transform::GetParent()
	{
		return m_parentTransform;
	}
}


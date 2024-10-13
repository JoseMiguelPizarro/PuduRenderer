#include "Transform.h"
#include <glm/ext/matrix_transform.hpp>
#include "PuduMath.h"

using namespace glm;

namespace Pudu {
	void Transform::SetRotation(quat r)
	{
		m_localRotation = degrees(eulerAngles(r));
	}

	void Transform::SetLocalRotationEuler(vec3 eulerAngles) {
		m_localRotation = eulerAngles;
	}


	vec3 Transform::GetLocalRotationEuler() {
		return m_localRotation;
	}

	quat Transform::GetRotationQuat()
	{
		return quat(radians(m_localRotation));
	}

	vec3 Transform::GetLocalPosition()
	{
		return m_localPosition;
	}

	void Transform::SetLocalPosition(vec3 position)
	{
		m_localPosition = position;

	}

	vec3 Transform::GetLocalScale() {
		return m_localScale;
	}

	void Transform::SetLocalScale(vec3 scale)
	{
		m_localScale = scale;
	}

	mat4 Transform::GetTransformationMatrix()
	{
		mat4 rot = toMat4(GetRotationQuat());
		return m_parentMatrix * translate(mat4(1.0f), m_localPosition) * scale(mat4(1.0f), m_localScale) * rot;
	}

	mat4 Transform::GetParentMatrix()
	{
		return m_parentMatrix;
	}

	void Transform::SetParentMatrix(mat4 m)
	{
		m_parentMatrix = m;
	}

	void Transform::SetForward(vec3 forward, vec3 up)
	{
		quat rot = Pudu::LookRotation(normalize(forward), normalize(up));

		//vec3 rotEuler = degrees(PuduMath::EulerAnglesFromQuat(rot));
		vec3 rotEuler = degrees(eulerAngles(rot));
		m_localRotation = rotEuler;
	}

	vec3 Transform::GetForward()
	{
		return normalize(rotate(GetRotationQuat(), vec3(0, 0, 1)));
	}
	void Transform::UpdateWorldTransformRecursivelly()
	{
		if (m_parentTransform != nullptr)
		{
			m_parentMatrix = m_parentTransform->GetTransformationMatrix();
		}
		else
		{
			m_parentMatrix = mat4(1.0f);
		}

		for (auto child : m_children) {
			child->UpdateWorldTransformRecursivelly();
		}
	}
	void Transform::SetParent(Transform* parent)
	{
		if (parent == nullptr)
		{
			UpdateWorldTransformRecursivelly();
		}
		else
		{
			m_parentTransform = parent;
			parent->AddChild(this);
		}
	}
	void Transform::AddChild(Transform* child)
	{
		m_children.push_back(child);
		UpdateWorldTransformRecursivelly();
	}

	Transform* Transform::GetParent()
	{
		return m_parentTransform;
	}


}


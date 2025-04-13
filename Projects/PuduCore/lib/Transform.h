#pragma once
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <vector>

using namespace glm;

namespace Pudu {
	struct Transform
	{
		void SetRotation(quat r);
		void SetLocalRotationEuler(vec3 eulerAngles);
		vec3 GetLocalRotationEuler();
		quat GetRotationQuat();

		vec3 GetLocalPosition();
		void SetLocalPosition(vec3 pos);

		vec3 GetLocalScale();
		void SetLocalScale(vec3 scale);
		void SetUniformLocalScale(f32 scale);

		mat4 GetTransformationMatrix();
		vec3 GetForward();
		void SetForward(vec3 forward, vec3 up);

		mat4 GetParentMatrix();
		void SetParentMatrix(mat4 m);

		void UpdateWorldTransformRecursivelly();
		void SetParent(Transform* parent);
		Transform* GetParent();
		std::vector<Transform>* GetChildren();


		/// <summary>
		/// Transform Changed event
		/// </summary>
		void(*TransformChanged)(Transform&);

	private:
		friend class ImGuiUtils;

		mat4 m_parentMatrix = mat4(1.0f);
		vec3 m_localRotation = vec3(0, 0, 0);
		vec3 m_localPosition = vec3(0, 0, 0);
		vec3 m_localScale = vec3(1, 1, 1);
		std::vector<Transform*> m_children;
		Transform* m_parentTransform = nullptr;
		void AddChild(Transform* child);
	};
}


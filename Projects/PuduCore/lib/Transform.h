#pragma once
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

using namespace glm;

struct Transform
{
	vec3 LocalPosition = vec3(0, 0, 0);
	vec3 LocalScale = vec3(1, 1, 1);

	/**
	 *Rotation in euler angles
	 */
	vec3 LocalRotation = vec3(0, 0, 0);

	quat GetRotationQuat();
	mat4 GetTransformationMatrix();
	void SetForward(vec3 forward, vec3 up);
	vec3 GetForward();
	mat4 ParentMatrix;

	void(*TransformChanged)(Transform&);
};

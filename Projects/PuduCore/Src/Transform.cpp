#include "Transform.h"
#include <glm/ext/matrix_transform.hpp>
#include "PuduMath.h"

using namespace glm;

quat Transform::GetRotationQuat()
{
	return quat(radians(LocalRotation));
}

mat4 Transform::GetTransformationMatrix()
{
	mat4 rot = toMat4(GetRotationQuat());
	return translate(mat4(1.0f), LocalPosition) * scale(mat4(1.0f), LocalScale) * rot;
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

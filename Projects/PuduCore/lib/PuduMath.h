#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define PI 3.141592653589

using namespace glm;

/*
<<VULKAN COORDINATE SYSTEM>>
Right hand system. +Z going inwards. Y going down

Z+
 .─────────►X+
 │
 │
 │
 │
 ▼
 Y+
 */

 //All matrices are in Column Major
namespace Pudu {

	static mat4 LookAt(vec3 eyePosition, vec3 targetPosition, vec3 up) {
		vec3 direction = normalize(targetPosition - eyePosition);

		vec3 w = direction;
		vec3 u = normalize(cross(up, w));
		vec3 v = cross(w, u);

		return	mat4{
			u.x,u.y,u.z,eyePosition.x,
			v.x,v.y,v.z,eyePosition.y,
			w.x,w.y,w.z,eyePosition.z,
			0,0,0,1 };
	}

	static mat4 PerspectiveMatrix(float vertical_fov, float aspect_ratio, float n, float f)
	{
		float fov_rad = vertical_fov * 2.0f * PI / 360.0f;
		float focal_length = 1.0f / std::tan(fov_rad / 2.0f);

		float x = focal_length * aspect_ratio;
		float A = f / (f - n);
		float B = -n * A;

		mat4 pm
		(
			aspect_ratio * focal_length, 0.0f, 0.0f, 0.0f,
			0.0f, -focal_length, 0.0f, 0.0f, //Multiply by -1 to remap to vk ndc space
			0.0f, 0.0f, A, 1,
			0.0f, 0.0f, B, 0.0f);

		return pm;
	}


	static mat4 LookAtInverse(vec3 eyePosition, vec3 forward, vec3 up)
	{
		//Right handed, since is the inverse the cross producs right and up are inverted
		vec3 forward_axis = forward;
		vec3 right_axis = normalize(cross(forward_axis, up)); 
		vec3 up_axis = cross(right_axis, forward_axis); 

		vec3 t = vec3(-dot(eyePosition, right_axis), -dot(eyePosition, up_axis), -dot(eyePosition, forward_axis));

		return	mat4{
			vec4(right_axis.x,up_axis.x,forward_axis.x,0.0f),
			vec4(right_axis.y,up_axis.y,forward_axis.y,0.0f),
			vec4(right_axis.z,up_axis.z,forward_axis.z,0.0f),
			vec4(-dot(eyePosition,right_axis),-dot(eyePosition,up_axis),-dot(eyePosition,forward_axis),1.0f) };
	}

	static quat LookRotation(vec3 forward, vec3 up)
	{
		vec3 right = normalize(cross(up, forward));
		return quat(mat3(right, cross(forward, right), forward));
	}

	/// <summary>
	/// Returns euler angles in radians, picht(X), yaw(y), roll(z)
	/// </summary>
	/// <param name="q"></param>
	/// <returns></returns>
	static vec3 EulerAnglesFromQuat(quat q) {
		mat3 r = toMat3(q); //Apparently glm handles rotation in Rz*Rx*Ry

		float pitch = asin(r[1][2]); //X axis
		float roll = atan2(-r[1][0], r[1][1]);//Z axis
		float yaw = atan2(-r[0][2], r[2][2]); //Y axis

		return vec3(pitch, yaw, roll);
	}

	struct Rect2DInt {
		i16                             x = 0;
		i16                             y = 0;
		u16                             width = 0;
		u16                             height = 0;
	};
}



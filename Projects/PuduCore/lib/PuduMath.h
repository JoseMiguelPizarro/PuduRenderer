#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.141592653589

using namespace glm;

/*
<<VULKAN COORDINATE SYSTEM>>
Right hand system. +Z going inwards

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
namespace PuduMath {

	static mat4 LookAt(vec3 eyePosition, vec3 targetPosition, vec3 up) {
		vec3 direction = normalize(targetPosition - eyePosition);

		vec3 w = direction;
		vec3 u = normalize(cross(up, w));
		vec3 v = cross(w, u);

		mat4 m;

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
			0.0f, focal_length, 0.0f, 0.0f, //Multiply by -1 to remap to vk ndc space
			0.0f, 0.0f, A, 1,
			0.0f, 0.0f, B, 0.0f);

		return pm;
	}

	static mat4 LookAtInverse(vec3 eyePosition, vec3 targetPosition, vec3 up)
	{
		vec3 forward_axis = normalize(targetPosition - eyePosition);;
		vec3 right_axis = normalize(cross(up, forward_axis));
		vec3 up_axis = cross(forward_axis, right_axis);

		vec3 t = vec3(-dot(eyePosition, right_axis), -dot(eyePosition, up_axis), -dot(eyePosition, forward_axis));

		return	mat4{
			vec4(right_axis.x,up_axis.x,forward_axis.x,0.0f),
			vec4(right_axis.y,up_axis.y,forward_axis.y,0.0f),
			vec4(right_axis.z,up_axis.z,forward_axis.z,0.0f),
			vec4(-dot(eyePosition,right_axis),-dot(eyePosition,up_axis),-dot(eyePosition,forward_axis),1.0f) };
	}
}



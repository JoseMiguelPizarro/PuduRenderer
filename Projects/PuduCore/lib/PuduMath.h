#pragma once
#include <hlsl++.h>

#define PI 3.141592653589

using namespace hlslpp;


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

	static float4x4 LookAt(float3 eyePosition, float3 targetPosition, float3 up) {
		float3 direction = normalize(targetPosition - eyePosition);

		float3 w = direction;
		float3 u = normalize(cross(up, w));
		float3 v = cross(w, u);

		float4x4 m;

		return	float4x4{
			u.x,u.y,u.z,eyePosition.x,
			v.x,v.y,v.z,eyePosition.y,
			w.x,w.y,w.z,eyePosition.z,
			0,0,0,1 };
	}

	static float4x4 PerspectiveMatrix(float vertical_fov, float aspect_ratio, float n, float f)
	{
		float fov_rad = vertical_fov * 2.0f * PI / 360.0f;
		float focal_length = 1.0f / std::tan(fov_rad / 2.0f);

		float x = focal_length * aspect_ratio;
		float A = f / (f - n);
		float B = -n * A;

		float4x4 pm
		(
			aspect_ratio * focal_length, 0.0f, 0.0f, 0.0f,
			0.0f, focal_length, 0.0f, 0.0f, //Multiply by -1 to remap to vk ndc space
			0.0f, 0.0f, A, 1,
			0.0f, 0.0f, B, 0.0f);

		return pm;
	}

	static float4x4 LookAtInverse(float3 eyePosition, float3 targetPosition, float3 up)
	{
		float3 forward_axis = normalize(targetPosition - eyePosition);;
		float3 right_axis = normalize(cross(up, forward_axis));
		float3 up_axis = cross(forward_axis, right_axis);

		float3 t = float3(-dot(eyePosition, right_axis), -dot(eyePosition, up_axis), -dot(eyePosition, forward_axis));

		return	float4x4{
			float4(right_axis.x,up_axis.x,forward_axis.x,0.0f),
			float4(right_axis.y,up_axis.y,forward_axis.y,0.0f),
			float4(right_axis.z,up_axis.z,forward_axis.z,0.0f),
			float4(-dot(eyePosition,right_axis),-dot(eyePosition,up_axis),-dot(eyePosition,forward_axis),1.0f) };
	}
}



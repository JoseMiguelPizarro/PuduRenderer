#pragma once
#include <hlsl++.h>

using namespace hlslpp;


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

	static float4x4 LookAtInverse(float3 eyePosition, float3 targetPosition, float3 up)
	{
		float3 w = normalize(targetPosition - eyePosition);;
		float3 u = normalize(cross(up, w));
		float3 v = cross(w, u);

		return	float4x4{
			float4(u.x,v.x,w.x,0.0f),
			float4(u.y,v.y,w.y,0.0f),
			float4(u.z,v.z,w.z,0.0f),
			float4(-dot(eyePosition,u),-dot(eyePosition,v),-dot(eyePosition,w),1.0f) };
	}
}



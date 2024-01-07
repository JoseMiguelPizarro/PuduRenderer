#pragma once
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

using namespace glm;

struct Transform
{
    vec3 Position = vec3(0, 0, 0);
    vec3 Scale = vec3(1, 1, 1);

    /**
     *Rotation in euler angles
     */
    vec3 Rotation = vec3(0, 0, 0);

    quat GetRotationQuat();
    mat4 GetTransformationMatrix();
    void SetForward(vec3 forward, vec3 up);
    vec3 GetForward();
};

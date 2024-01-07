#pragma once
#include "Transform.h"

using namespace glm;

class Camera
{
public:
   Transform Transform;
   float Width;
   float Height;
   float Fov = 45;

   mat4 GetViewMatrix();
   mat4 GetInverseViewMatrix();
   mat4 GetPerspectiveMatrix();
};

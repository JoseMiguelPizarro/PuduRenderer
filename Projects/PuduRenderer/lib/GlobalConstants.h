#pragma once
#include "glm/glm.hpp"

struct alignas(16) GlobalConstants
{
      vec2 screenSize;
      float nearPlane;
      float farPlane;
      vec3 cameraPosWS;
      float time;
      mat4 viewMatrix;
      mat4 projectionMatrix;
};
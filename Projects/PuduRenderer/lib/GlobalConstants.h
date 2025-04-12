#pragma once
#include "glm/glm.hpp"

struct  GlobalConstants
{
      alignas(8) vec2 screenSize;
      alignas(4) float time;
      alignas(4) float nearPlane;
      alignas(4) float farPlane;
      alignas(16) vec3 cameraPosWS;
      alignas(16) mat4 viewMatrix;
      alignas(16) mat4 projectionMatrix;
};
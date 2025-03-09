#pragma once
#include "glm/glm.hpp"

struct GlobalConstants
{
      glm::vec2 screenSize;
      float nearPlane;
      float farPlane;
      glm::vec3 cameraPosWS;
      float time;
      mat4 viewMatrix;
      mat4 projectionMatrix;
};
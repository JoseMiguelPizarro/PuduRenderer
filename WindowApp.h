#pragma once
#include <GLFW/glfw3.h>
class WindowApp {
public:
	GLFWwindow* m_windowPtr;
	bool FrameBufferResized = false;
};
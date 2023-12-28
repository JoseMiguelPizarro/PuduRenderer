#include "PuduApp.h"

void PuduApp::Init()
{
	printf("PuduApp Init");
	Graphics.Init(800, 600);
}
void PuduApp::Cleanup()
{
	vkDeviceWaitIdle(Graphics.m_device);
	Graphics.Cleanup();

	OnCleanup();
}
void PuduApp::Run() {
	printf("PuduApp Run");
	while (!glfwWindowShouldClose(Graphics.m_windowPtr)) {
		glfwPollEvents();

		OnRun();
	}

	Cleanup();
}

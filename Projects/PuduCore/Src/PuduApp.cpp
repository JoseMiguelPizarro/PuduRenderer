#include "PuduApp.h"

void PuduApp::Init()
{
	printf("PuduApp Init");
	Graphics.Init(800, 600);
}
void PuduApp::Cleanup()
{
	vkDeviceWaitIdle(Graphics.Device);
	Graphics.Cleanup();

	OnCleanup();
}
void PuduApp::Run() {
	printf("PuduApp Run");
	while (!glfwWindowShouldClose(Graphics.WindowPtr)) {
		glfwPollEvents();

		OnRun();
	}

	Cleanup();
}

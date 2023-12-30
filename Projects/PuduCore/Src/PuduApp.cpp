#include "PuduApp.h"
#include <chrono>
#include <format>
#include <Logger.h>
#include <thread>

using namespace std::chrono_literals;

void PuduApp::Init()
{
	printf("PuduApp Init");
	Graphics.Init(800, 600);

	lastFrameTime = std::chrono::high_resolution_clock::now();
}
void PuduApp::Cleanup()
{
	vkDeviceWaitIdle(Graphics.Device);
	Graphics.Cleanup();

	OnCleanup();
}
void PuduApp::Run() {

	float targetFrameDuration = 1.0f / TargetFPS;

	Print("PuduApp Run");
	while (!glfwWindowShouldClose(Graphics.WindowPtr)) {
		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> frameDuration = currentTime - lastFrameTime;

		float durationDelta = targetFrameDuration - frameDuration.count();
		if (durationDelta > 0)
		{
			std::this_thread::sleep_for(std::chrono::duration<float>(durationDelta));
		}

		glfwPollEvents();
		OnRun();
		lastFrameTime = std::chrono::high_resolution_clock::now();
	}

	Cleanup();
}

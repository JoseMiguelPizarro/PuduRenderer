#include <chrono>
#include <format>
#include <thread>
#include "Logger.h"
#include "PuduApp.h"

namespace Pudu
{
	void PuduApp::Init()
	{
		printf("PuduApp Init");
		Graphics.Init(1024, 1024);
		OnInit();

		Time.m_startTime = std::chrono::high_resolution_clock::now();
		Time.m_lastFrameTime = std::chrono::high_resolution_clock::now();
	}

	void PuduApp::Cleanup()
	{
		Graphics.WaitIdle();
		OnCleanup();
		Graphics.Cleanup();
	}

	void PuduApp::Run()
	{
		float targetFrameDuration = 1.0f / TargetFPS;

		Print("PuduApp Run");
		while (!glfwWindowShouldClose(Graphics.WindowPtr))
		{
			Time.m_currentFrameTime = std::chrono::high_resolution_clock::now();
			float deltaTime = Time.DeltaTime();
			float durationDelta = targetFrameDuration - deltaTime * 2; //Multiply by 2 to compensate for frame next to render
			durationDelta = durationDelta < 0 ? 0 : durationDelta;

			if (durationDelta > 0)
			{
				std::this_thread::sleep_for(std::chrono::duration<float>(durationDelta));
			}

			Time.m_currentFrameTime = std::chrono::high_resolution_clock::now();

			glfwPollEvents();
			OnRun();
			Time.m_lastFrameTime = Time.m_currentFrameTime;
		}

		Cleanup();
	}
}

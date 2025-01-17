#include <chrono>
#include <format>
#include <thread>
#include "Logger.h"
#include "PuduApp.h"
#include <iostream>
#include <exception>



namespace Pudu
{
	void PuduApp::Init()
	{
		LOG("PuduApp Init");
		Graphics.Init(1024, 1024);
		OnInit();

		Time.m_startTime = std::chrono::high_resolution_clock::now();
		Time.m_endFrameTime = std::chrono::high_resolution_clock::now();
		Graphics.SetTime(&Time);
	}

	void PuduApp::Cleanup()
	{
		Graphics.WaitIdle();
		OnCleanup();
		Graphics.Cleanup();
	}

	void PuduApp::Run()
	{

		auto targetFrameDuration = std::chrono::duration<float, std::milli>(1000.0f / (float)TargetFPS);
		Time.m_startFrameTime = std::chrono::high_resolution_clock::now();
		Time.m_endFrameTime = std::chrono::high_resolution_clock::now();

		//Main Loop
		while (!glfwWindowShouldClose(Graphics.WindowPtr))
		{
			Time.m_startFrameTime = std::chrono::high_resolution_clock::now();
			auto startFrame = std::chrono::high_resolution_clock::now();
			glfwPollEvents();

			try {

				OnRun();
			}
			catch (std::exception& e) {
				std::printf(e.what());
			}

			std::chrono::duration<float, std::milli> elapsed = std::chrono::high_resolution_clock::now() - Time.m_startFrameTime;

			//If load is to low, let's artificially wait before continuing
			if (elapsed.count() < 2.0f)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));

				elapsed = std::chrono::high_resolution_clock::now() - Time.m_startFrameTime;
			}

				if (elapsed < targetFrameDuration)
			{
				auto towait = targetFrameDuration - elapsed * 2; //Multiplied by 2 to compensate to next frame

				std::this_thread::sleep_for(towait);
			}

			Time.m_endFrameTime = std::chrono::high_resolution_clock::now();

			std::chrono::duration<float, std::milli> deltaTime = std::chrono::high_resolution_clock::now() - startFrame;
			Time.m_deltaTime = deltaTime.count();
		}

		Cleanup();
	}
}

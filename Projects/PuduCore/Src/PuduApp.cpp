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
		PuduGraphicsSettings settings;
		settings.resolution.x = 1024;
		settings.resolution.y = 1024;
		settings.presentMode = PresentMode::FIFO;
		Graphics.Init(settings);
		Input::Init(Graphics.WindowPtr);

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

		Time.m_startFrameTime = std::chrono::high_resolution_clock::now();
		Time.m_endFrameTime = std::chrono::high_resolution_clock::now();

		//Main Loop
		while (!glfwWindowShouldClose(Graphics.WindowPtr))
		{
			glfwPollEvents();
			auto targetFrameDuration = std::chrono::duration<double, std::milli>(1000.0f / static_cast<double>(TargetFPS));
			Time.m_startFrameTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed = std::chrono::duration<double, std::milli>( Time.m_deltaTime);

			if (elapsed < targetFrameDuration)
			{
				std::chrono::duration<double, std::milli> timeToWait = targetFrameDuration;
				std::this_thread::sleep_for(timeToWait);
			}
			try
			{
				OnRun();
				Input::ResetDelta();
			}
			catch (std::exception& e)
			{
				std::printf(e.what());
			}

			Time.m_endFrameTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> deltaTime = Time.m_endFrameTime - Time.m_startFrameTime;
			Time.m_deltaTime = deltaTime.count();
		}

		Cleanup();
	}
}

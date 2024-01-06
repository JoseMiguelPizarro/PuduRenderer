#pragma once
#include <PuduGraphics.h>
#include <chrono>

namespace  Pudu
{
	class PuduApp
	{
	public:
		virtual ~PuduApp() = default;
		void Run();
		void Init();

		bool FrameBufferResized;
		float TargetFPS = 60;
		PuduGraphics Graphics;

	private:
		void Cleanup();

		void virtual OnRun()
		{
		};

		virtual void OnInit() {};


		void virtual OnCleanup()
		{
		};

		std::chrono::steady_clock::time_point lastFrameTime;
	};

}

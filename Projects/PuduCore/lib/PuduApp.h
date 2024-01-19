#pragma once
#include <chrono>
#include "PuduGraphics.h"
#include "PuduTime.h"

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
		PuduTime Time;

	private:
		void Cleanup();

		void virtual OnRun()
		{
		};

		virtual void OnInit() {};


		void virtual OnCleanup()
		{
		};
	};

}

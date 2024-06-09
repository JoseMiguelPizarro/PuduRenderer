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
		int TargetFPS = 30;
		PuduGraphics Graphics;
		PuduTime Time;


		void virtual DrawImGUI() {};

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

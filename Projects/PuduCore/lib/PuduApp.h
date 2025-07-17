#pragma once
#include <chrono>

#include "FileManager.h"
#include "Input.h"
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
		UPtr<PuduGraphics> Graphics;
		UPtr<EntityManager> EntityManager;
		UPtr<PuduTime> Time;

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

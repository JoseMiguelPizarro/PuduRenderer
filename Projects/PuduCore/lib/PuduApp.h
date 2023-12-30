#pragma once
#include <PuduGraphics.h>
#include <chrono>


class PuduApp {
public:
	void Init();
	void Run();

	bool FrameBufferResized;
	float TargetFPS = 60;
	PuduGraphics Graphics;
private:
	void Cleanup();
	void  virtual OnRun() {};
	void  virtual OnCleanup() {};

	std::chrono::steady_clock::time_point lastFrameTime;
};
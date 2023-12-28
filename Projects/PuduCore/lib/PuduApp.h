#pragma once
#include <PuduGraphics.h>

class PuduApp {
public:
	void Init();
	void Run();

	bool FrameBufferResized;
	PuduGraphics Graphics;
private:
	void Cleanup();
	void  virtual OnRun() {};
	void  virtual OnCleanup() {};
};
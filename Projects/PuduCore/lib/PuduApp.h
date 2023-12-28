#pragma once
#include <PuduGraphics.h>

class PuduApp {
public:
	bool FrameBufferResized;
	void Init();
	void Run();
	PuduGraphics Graphics;
private:
	void Cleanup();
	void  virtual OnRun() {};
	void  virtual OnCleanup() {};
};
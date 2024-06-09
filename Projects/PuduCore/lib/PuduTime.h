#pragma once

#include <chrono>

namespace Pudu {
	typedef std::chrono::steady_clock::time_point timePoint;

	class PuduTime
	{
	public:
		timePoint GetStartTime() {
			return m_startTime;
		}

		timePoint GetLastFrameTime() {
			return m_endFrameTime;
		}

		float Time() {
			return std::chrono::duration<float, std::chrono::seconds::period>(m_startFrameTime - m_startTime).count();
		}

		timePoint GetCurrentFrameTime() {
			return m_startFrameTime;
		}

		float GetFPS() {
			return 1000.f / DeltaTime();
		}

		float DeltaTime() {
			return m_deltaTime;
		}

	private:
		friend class PuduApp;
		timePoint m_startTime;
		timePoint m_endFrameTime;
		timePoint m_startFrameTime;
		float m_deltaTime;
	};
}


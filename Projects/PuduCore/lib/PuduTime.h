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
			return m_lastFrameTime;
		}

		float Time() {
			return std::chrono::duration<float, std::chrono::seconds::period>(m_currentFrameTime - m_startTime).count();
		}

		timePoint GetCurrentFrameTime() {
			return m_currentFrameTime;
		}

		float GetFPS() {
			return 1 / DeltaTime();
		}

		float DeltaTime() {
			return std::chrono::duration<float>(m_currentFrameTime - m_lastFrameTime).count();
		}

	private:
		friend class PuduApp;
		timePoint m_startTime;
		timePoint m_lastFrameTime;
		timePoint m_currentFrameTime;
	};
}


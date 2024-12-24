#include "Semaphore.h"


namespace Pudu
{
	uint64_t Semaphore::Signal()
	{
		return ++m_timelineValue;
	}

	uint64_t Semaphore::TimelineValue()
	{
		return m_timelineValue;
	}
}

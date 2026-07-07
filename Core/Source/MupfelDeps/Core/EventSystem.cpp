#include "EventSystem.h"

Mupfel::EventSystem::EventSystem() : current(0), next(1)
{
}

void Mupfel::EventSystem::Update()
{
	/* Tick is over, clear the current EventBuffers */
	for (auto& e : event_buffer_array[current])
	{
		if (e)
		{
			e->Clear();
		}
	}

	/* Update the index */
	std::swap(current, next);

	/* Update the event counters */
	events_last_frame = events_this_frame;
	events_this_frame = 0;
}

uint64_t Mupfel::EventSystem::GetLastEventCount() const
{
	return events_last_frame;
}

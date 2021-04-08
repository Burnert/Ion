#pragma once

#include "Event.h"

namespace Ion
{
	class ION_API EventQueue
	{
		using EventHandler = std::function<void(Event&)>;
	public:
		void PushEvent(DeferredEvent&& event);
		bool ProcessEvents();

		void SetEventHandler(EventHandler handler);

	private:
		std::deque<DeferredEvent> m_DeferredEvents;

		EventHandler m_Handler;
	};
}

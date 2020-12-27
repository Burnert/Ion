#pragma once

#include "Event.h"

namespace Ion
{
	class ION_API EventQueue
	{
		using EventHandler = std::function<void(Event&)>;
		using EventPtr = std::shared_ptr<Event>;

	public:
		void PushEvent(EventPtr event);
		bool ProcessEvents();

		void SetEventHandler(EventHandler handler);

	private:
		std::vector<EventPtr> m_Events;

		EventHandler m_Handler;
	};
}

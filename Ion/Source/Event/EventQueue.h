#pragma once

#include "Event.h"

namespace Ion
{
	class ION_API EventQueue
	{
		using EventHandler = std::function<void(Event&)>;
		using EventPtr = std::shared_ptr<Event>;
	public:
		template<typename EventT, typename std::enable_if<std::is_base_of<Event, EventT>::value>::type* = nullptr>
		void PushEvent(std::shared_ptr<EventT> event)
		{
			m_Events.push_back(event);
		}

		bool ProcessEvents();

		void SetEventHandler(EventHandler handler);

	private:
		std::vector<EventPtr> m_Events;

		EventHandler m_Handler;
	};
}

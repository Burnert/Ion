#pragma once

#include "Event.h"
#include <unordered_map>

namespace Ion
{
	class ION_API EventQueue
	{
		using EventHandler = std::function<void(Event&)>;
		using EventRef = std::reference_wrapper<Event>;

	public:
		template<typename EventT, typename std::enable_if<std::is_base_of<Event, EventT>::value>::type* = nullptr>
		void PushEvent(EventT& event)
		{
			m_Events.push_back(event);
		}

		bool ProcessEvents();

		void SetEventHandler(EventHandler handler);

	private:
		std::vector<EventRef> m_Events;

		EventHandler m_Handler;
	};
}

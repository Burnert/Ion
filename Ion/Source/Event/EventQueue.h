#pragma once

#include "Event.h"

namespace Ion
{
	class ION_API EventQueue
	{
	public:
		template<typename EventT, typename std::enable_if<std::is_base_of<Event, EventT>::value>::type* = nullptr>
		void PushEvent(EventT& event)
		{
			m_Events.push_back(std::make_shared<EventT>(event));
		}

		bool ProcessEvents();

	private:
		std::vector<std::shared_ptr<Event>> m_Events;
	};
}

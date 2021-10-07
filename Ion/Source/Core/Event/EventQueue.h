#pragma once

#include "Event.h"

namespace Ion
{
	template<void Handler(const Event&)>
	class ION_API EventQueue
	{
	public:
		template<typename EventT>
		void PushEvent(const EventT& event)
		{
			m_Events.emplace_back<EventT>(event);
		}

		bool ProcessEvents()
		{
			TRACE_FUNCTION();

			if (!m_Events.empty())
			{
				for (Event* e : m_Events)
				{
					Handler(e);
				}

				m_Events.clear();
				return true;
			}
			return false;
		}

	private:
		TArray<Event*> m_Events;
	};
}

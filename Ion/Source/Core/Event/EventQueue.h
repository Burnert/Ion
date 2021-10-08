#pragma once

#include "Event.h"

namespace Ion
{
	template<void Handler(const Event&)>
	class EventQueue
	{
	public:
		template<typename EventT>
		void PushEvent(const EventT& event)
		{
			Event* e = new EventT(event);
			m_Events.push_back(e);
		}

		bool ProcessEvents()
		{
			TRACE_FUNCTION();

			if (!m_Events.empty())
			{
				for (int i = 0; i < m_Events.size(); ++i)
				{
					const Event* e = m_Events[i];
					Handler(*e);
				}

				Clear();
				return true;
			}
			return false;
		}

		void Clear()
		{
			for (int i = 0; i < m_Events.size(); ++i)
			{
				Event* e = m_Events[i];
				delete e;
			}
			m_Events.clear();
		}

	private:
		TArray<Event*> m_Events;
	};
}

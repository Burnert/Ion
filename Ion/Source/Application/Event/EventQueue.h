#pragma once

#include "Event.h"

namespace Ion
{
	class EventQueue
	{
	public:
		void PushEvent(const Event& e);

		template<typename F>
		void ProcessEvents(F handler);

		void Clear();

	private:
		TArray<std::unique_ptr<const Event>> m_Events;
	};

	inline void EventQueue::PushEvent(const Event& e)
	{
		m_Events.push_back(e.Defer());
	}

	template<typename F>
	inline void EventQueue::ProcessEvents(F handler)
	{
		TRACE_FUNCTION();

		for (std::unique_ptr<const Event>& e : m_Events)
		{
			handler(*e);
		}
		Clear();
	}

	inline void EventQueue::Clear()
	{
		m_Events.clear();
	}
}

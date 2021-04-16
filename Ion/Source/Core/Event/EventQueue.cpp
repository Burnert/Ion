#include "IonPCH.h"

#include "EventQueue.h"
#include "Core/Logging/Logger.h"
#include "Core/CoreUtility.h"

namespace Ion
{
	void EventQueue::PushEvent(DeferredEvent&& event)
	{
		m_DeferredEvents.push_back(event);
	}

	bool EventQueue::ProcessEvents()
	{
		TRACE_FUNCTION();

		if (!m_DeferredEvents.empty())
		{
			//ION_LOG_ENGINE_TRACE("Processing events in queue.");

			for (DeferredEvent deferredEvent : m_DeferredEvents)
			{
				Event& event = *deferredEvent;
				m_Handler(event);
			}

			m_DeferredEvents.clear();
			return true;
		}
		return false;
	}

	void EventQueue::SetEventHandler(EventHandler handler)
	{
		m_Handler = handler;
	}
}

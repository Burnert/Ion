#include "IonPCH.h"

#include "EventQueue.h"
#include "Core/Logging/Logger.h"
#include "Core/CoreUtilities.h"

namespace Ion
{
	void EventQueue::PushEvent(EventPtr event)
	{
		m_Events.push_back(event);
	}

	bool EventQueue::ProcessEvents()
	{
		if (!m_Events.empty())
		{
			ION_LOG_ENGINE_TRACE("Processing events in queue.");

			for (auto it = m_Events.begin(); it != m_Events.end(); ++it)
			{
				Event& event = **it;
				m_Handler(event);
			}

			m_Events.clear();
			return true;
		}
		return false;
	}

	void EventQueue::SetEventHandler(EventHandler handler)
	{
		m_Handler = handler;
	}
}

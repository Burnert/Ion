#pragma once

#include "Event.h"

namespace Ion
{
	class EventDispatcher
	{
		template<typename T, typename std::enable_if<std::is_base_of<Event, T>::value>::type* = nullptr>
		using EventFunc = std::function<bool(T&)>;

	public:
		EventDispatcher(Event& event) :
			m_Event(event) {}

		template<typename T, typename std::enable_if<std::is_base_of<Event, T>::value>::type* = nullptr>
		bool Dispatch(EventFunc<T> callback)
		{
			if (m_Event.GetType() == T::GetStaticType())
			{
				m_Event.m_Handled = callback(*(T*)&m_Event);
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};
}

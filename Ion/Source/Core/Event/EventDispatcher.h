#pragma once

#include "Event.h"

namespace Ion
{
	// @TODO: EventDispatcher needs some work

	class ION_API EventDispatcher
	{
		template<typename T, typename TEnableIfT<TIsBaseOfV<Event, T>>* = nullptr>
		using EventFunc = std::function<bool(T&)>;

	public:
		EventDispatcher(Event& event) :
			m_Event(event) {}

		template<typename T, typename TEnableIfT<TIsBaseOfV<Event, T>>* = nullptr>
		bool Dispatch(EventFunc<T> callback)
		{
			if (m_Event.GetType() == T::GetStaticType())
			{
				m_Event.m_bHandled = callback(*(T*)&m_Event);
				return true;
			}
			return false;
		}

		FORCEINLINE bool Handled() const { return m_Event.m_bHandled; }

	private:
		Event& m_Event;
	};
}

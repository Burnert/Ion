#pragma once

#include "Event.h"

namespace Ion
{
	// @TODO: EventDispatcher needs some work

	class ION_API EventDispatcher
	{
		template<typename T>
		using EventCallback = TFunction<void(T&)>;

	public:
		EventDispatcher(Event& event) { }

		template<typename T>
		void Bind(EventCallback<T> callback)
		{
			m_Callbacks.insert(T::_GetType(), callback)
		}

		template<typename T>
		void Dispatch(const T& event)
		{
			// nie wiem co tu sie odpitala
			void (*callback)(const T&);
		}

	private:
		THashMap<EEventType, void*> m_Callbacks;
	};
}

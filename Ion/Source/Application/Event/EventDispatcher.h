#pragma once

#include "Event.h"

namespace Ion
{
#pragma region Event Function Pointer Wrapper

	template<typename TClass, typename TEvent>
	using TEventMemberFunctionPointer = void(TClass::*)(const TEvent&);

	template<typename TClass>
	class TIEventFunctionWrapper
	{
	public:
		virtual void Call(TClass* object, const Event& e) = 0;
	};

	template<typename TClass, typename TEvent>
	class TEventFunctionWrapper : public TIEventFunctionWrapper<TClass>
	{
	public:
		TEventFunctionWrapper(TEventMemberFunctionPointer<TClass, TEvent> func);

		virtual void Call(TClass* object, const Event& e) override;

	private:
		TEventMemberFunctionPointer<TClass, TEvent> m_Function;
	};

	#pragma region TEventFunctionWrapper Implementation

	template<typename TClass, typename TEvent>
	inline TEventFunctionWrapper<TClass, TEvent>::TEventFunctionWrapper(TEventMemberFunctionPointer<TClass, TEvent> func) :
		m_Function(func)
	{
		ionassert(func);
	}

	template<typename TClass, typename TEvent>
	inline void TEventFunctionWrapper<TClass, TEvent>::Call(TClass* object, const Event& e)
	{
		ionassert(object);
		(object->*m_Function)(static_cast<const TEvent&>(e));
	}

	#pragma endregion

#pragma endregion

#pragma region Event Dispatcher

	/**
	 * @brief Dynamic, automated dispatcher for native events.
	 * 
	 * @tparam TClass Listener class (a class which has event callback functions)
	 */
	template<typename TClass>
	class TEventDispatcher
	{
	public:
		/**
		 * @brief Create an Event Dispatcher with a specified listener object.
		 * 
		 * @param listener 
		 */
		TEventDispatcher(TClass* listener);

		/**
		 * @brief Register a function as an event callback.
		 * 
		 * @tparam TEvent Event type to dispatch to the callback function
		 * @param func The callback member function pointer
		 */
		template<typename TEvent, TEnableIfT<TIsBaseOfV<Event, TEvent>>* = 0>
		void RegisterEventFunction(TEventMemberFunctionPointer<TClass, TEvent> func);

		/**
		 * @brief Unregister an event callback function.
		 * 
		 * @tparam TEvent Event type to dispatch to the callback function
		 * @param func The callback member function pointer
		 */
		template<typename TEvent, TEnableIfT<TIsBaseOfV<Event, TEvent>>* = 0>
		void UnregisterEventFunction(TEventMemberFunctionPointer<TClass, TEvent> func);

		/**
		 * @brief Dispatch an event to a previously registered
		 * callback function which takes a proper event type.
		 * 
		 * @param e Generic Event reference (the actual type will be resolved automatically)
		 */
		void Dispatch(const Event& e);

	private:
		TClass* m_Listener;
		TFixedArray<std::unique_ptr<TIEventFunctionWrapper<TClass>>, (size_t)EEventType::_Count> m_EventFunctions;
	};

	#pragma region TEventDispatcher Implementation

	template<typename TClass>
	TEventDispatcher<TClass>::TEventDispatcher(TClass* listener) :
		m_Listener(listener),
		m_EventFunctions()
	{
		ionassert(m_Listener, "Event Listener cannot be null.");
	}

	template<typename TClass>
	template<typename TEvent, TEnableIfT<TIsBaseOfV<Event, TEvent>>*>
	inline void TEventDispatcher<TClass>::RegisterEventFunction(TEventMemberFunctionPointer<TClass, TEvent> func)
	{
		EEventType type = TEvent::_GetType();

		ionassert(!m_EventFunctions[(size_t)type], "Event function for that event type has already been registered.");
		m_EventFunctions[(size_t)type] = std::make_unique<TEventFunctionWrapper<TClass, TEvent>>(func);
	}

	template<typename TClass>
	template<typename TEvent, TEnableIfT<TIsBaseOfV<Event, TEvent>>*>
	inline void TEventDispatcher<TClass>::UnregisterEventFunction(TEventMemberFunctionPointer<TClass, TEvent> func)
	{
		EEventType type = TEvent::_GetType();

		m_EventFunctions[(size_t)type] = nullptr;
	}

	template<typename TClass>
	inline void TEventDispatcher<TClass>::Dispatch(const Event& e)
	{
		EEventType type = e.GetType();

		if (m_EventFunctions[(size_t)type])
			m_EventFunctions[(size_t)type]->Call(m_Listener, e);
	}

	#pragma endregion

#pragma endregion
}

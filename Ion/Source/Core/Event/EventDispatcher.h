#pragma once

#include "Event.h"
#include "Core/Diagnostics/Tracing.h"

namespace Ion
{
	template<typename EventT, void Func(const EventT&)>
	struct TEventFunction
	{
		using EventType = EventT;

		static void Call(const EventT& event)
		{
			(*Func)(event);
		}
	};

	template<typename ClassT, typename EventT, void (ClassT::*Func)(const EventT&)>
	struct TMemberEventFunction
	{
		using EventType = EventT;
		
		static void Call(ClassT* object, const EventT& event)
		{
			(object->*Func)(event);
		}
	};

	template<typename... Functions>
	struct TEventFunctionPack;

	template<>
	struct TEventFunctionPack<> { };

	template<typename First, typename... Other>
	struct TEventFunctionPack<First, Other...> : private TEventFunctionPack<Other...>
	{
		using ThisFunction = First;
		using OtherFunctions = TEventFunctionPack<Other...>;
	};

	template<typename EventFunctions, typename ClassT = void>
	class EventDispatcher
	{
	public:
		EventDispatcher(ClassT* owner)
			: m_Owner(owner)
		{ }

		template<typename EventT>
		void Dispatch(const EventT& event)
		{
			TRACE_FUNCTION();

			DispatchExpand<EventT, EventFunctions>(event);
		}

		// Runtime version of Dispatch (used when the event type is not known at compile time)
		void Dispatch(const Event& event)
		{
			TRACE_FUNCTION();

			DispatchExpandRuntime<EventFunctions>(event);
		}

	private:
		// Member function version
		template<typename EventT, typename FuncPack>
		void DispatchExpand(const EventT& event)
		{
			// Find the correct type
			if constexpr (TIsSameV<FuncPack::ThisFunction::EventType, EventT>)
			{
				FuncPack::ThisFunction::Call(m_Owner, event);
			}
			else if constexpr (!TIsSameV<FuncPack::OtherFunctions, TEventFunctionPack<>>)
			{
				// Recursive template iterate if possible
				DispatchExpand<EventT, FuncPack::OtherFunctions>(event);
			}
		}

		template<typename FuncPack>
		void DispatchExpandRuntime(const Event& event)
		{
			// Find the correct type at runtime
			if (FuncPack::ThisFunction::EventType::_GetType() == event.GetType())
			{
				// Cast to the correct type (based on EventDispatcher template arguments)
				FuncPack::ThisFunction::EventType* eventPtr = (FuncPack::ThisFunction::EventType*)&event;
				FuncPack::ThisFunction::Call(m_Owner, *eventPtr);
			}
			else if constexpr (!TIsSameV<FuncPack::OtherFunctions, TEventFunctionPack<>>)
			{
				// Recursive template iterate if possible
				DispatchExpandRuntime<FuncPack::OtherFunctions>(event);
			}
		}

		ClassT* m_Owner;
	};

	template<typename EventFunctions>
	class EventDispatcher<EventFunctions, void>
	{
		template<typename EventT>
		void Dispatch(const EventT& event)
		{
			TRACE_FUNCTION();

			DispatchExpand<EventT, EventFunctions>(event);
		}

	private:
		// Non-member function version
		template<typename EventT, typename FuncPack>
		void DispatchExpand(const EventT& event)
		{
			// Find the correct type
			if constexpr (TIsSameV<FuncPack::ThisFunction::EventType, EventT>)
			{
				FuncPack::ThisFunction::Call(event);
			}
			else if constexpr (!TIsSameV<FuncPack::OtherFunctions, TEventFunctionPack<>>)
			{
				// Recursive template iterate if possible
				DispatchExpand<EventT, FuncPack::OtherFunctions>(event);
			}
		}
	};

}

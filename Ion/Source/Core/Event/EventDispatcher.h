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

	// @TODO: Make 2 specializations of this class for an in class member function dispatcher and a non class one
	// because writing "this" everywhere in Dispatch functions is really ugly

	template<typename EventFunctions>
	class EventDispatcher
	{
		template<typename T>
		using EventCallback = TFunction<void(T&)>;

	public:
		EventDispatcher() { }

		template<typename EventT>
		void Dispatch(const EventT& event)
		{
			TRACE_FUNCTION();

			DispatchExpand<EventT, EventFunctions>(event);
		}

		template<typename ClassT, typename EventT>
		void Dispatch(ClassT* object, const EventT& event)
		{
			TRACE_FUNCTION();

			DispatchExpand<ClassT, EventT, EventFunctions>(object, event);
		}

		// Runtime version of Dispatch (used when the event type is not known at compile time)
		template<typename ClassT>
		void Dispatch(ClassT* object, const Event& event)
		{
			TRACE_FUNCTION();

			DispatchExpandRuntime<ClassT, EventFunctions>(object, event);
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

		// Member function version
		template<typename ClassT, typename EventT, typename FuncPack>
		void DispatchExpand(ClassT* object, const EventT& event)
		{
			// Find the correct type
			if constexpr (TIsSameV<FuncPack::ThisFunction::EventType, EventT>)
			{
				FuncPack::ThisFunction::Call(object, event);
			}
			else if constexpr (!TIsSameV<FuncPack::OtherFunctions, TEventFunctionPack<>>)
			{
				// Recursive template iterate if possible
				DispatchExpand<ClassT, EventT, FuncPack::OtherFunctions>(object, event);
			}
		}

		template<typename ClassT, typename FuncPack>
		void DispatchExpandRuntime(ClassT* object, const Event& event)
		{
			// Find the correct type at runtime
			if (FuncPack::ThisFunction::EventType::_GetType() == event.GetType())
			{
				// Cast to the correct type (based on EventDispatcher template arguments)
				FuncPack::ThisFunction::EventType* eventPtr = (FuncPack::ThisFunction::EventType*)&event;
				FuncPack::ThisFunction::Call(object, *eventPtr);
			}
			else if constexpr (!TIsSameV<FuncPack::OtherFunctions, TEventFunctionPack<>>)
			{
				// Recursive template iterate if possible
				DispatchExpandRuntime<ClassT, FuncPack::OtherFunctions>(object, event);
			}
		}
	};
}

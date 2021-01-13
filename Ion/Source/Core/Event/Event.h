#pragma once

#include "Core/CoreMacros.h"
#include "Core/CoreTypes.h"
#include "Core/CoreApi.h"
#include "Core/CoreUtility.h"
#include "Core/Logging/Logger.h"

namespace Ion
{
	enum class EEventType
	{
		None = 0,

		// Window

		WindowClose,
		WindowResize,
		WindowFocus,
		WindowLostFocus,
		WindowMoved,

		// Engine

		EngineTick,
		EngineRender,

		// Keyboard

		KeyPressed,
		KeyReleased,
		KeyRepeated,

		// Mouse Button

		MouseButtonPressed,
		MouseButtonReleased,
		MouseDoubleClick,
		RawInputMouseButtonPressed,
		RawInputMouseButtonReleased,

		// Mouse

		MouseMoved,
		MouseScrolled,
		RawInputMouseMoved,
		RawInputMouseScrolled,
	};

	enum EEventCategory : uint
	{
		EC_None        = 0,
		EC_Application = Bitflag(0),
		EC_Window      = Bitflag(1),
		EC_Engine      = Bitflag(2),
		EC_Input       = Bitflag(3),
		EC_Keyboard    = Bitflag(4),
		EC_Mouse       = Bitflag(5),
		EC_MouseButton = Bitflag(6),
		EC_RawInput    = Bitflag(7),
	};

#ifdef ION_LOG_ENABLED
	#define SET_EVENT_TYPE(type) \
	static EEventType GetStaticType() { return EEventType::##type; } \
	FORCEINLINE virtual EEventType GetType() const override { return GetStaticType(); } \
	FORCEINLINE virtual const char* Debug_GetName() const override { return (#type"Event"); }
#else
	#define SET_EVENT_TYPE(type) \
	static EEventType GetStaticType() { return EEventType::##type; } \
	FORCEINLINE virtual EEventType GetType() const override { return GetStaticType(); }
#endif

#define SET_EVENT_CATEGORY(category) \
	FORCEINLINE virtual uint GetCategoryFlags() const override { return category; }

#ifdef ION_LOG_ENABLED // Debug / Release
	#define SET_EVENT_TOSTRING_FORMAT(format) \
	FORCEINLINE virtual std::string Debug_ToString() const override \
	{ \
		std::stringstream ss; \
		ss << Debug_GetName() << ": " << format; \
		return ss.str(); \
	}
#else
	#define SET_EVENT_TOSTRING_FORMAT(format)
#endif

	// Classes

	class ION_API Event
	{
		friend class EventDispatcher;

	public:
		FORCEINLINE virtual EEventType GetType() const = 0;
		FORCEINLINE virtual uint GetCategoryFlags() const = 0;

		FORCEINLINE bool IsInCategory(EEventCategory category) const
		{
			return GetCategoryFlags() & (uint)category;
		}

#ifdef ION_LOG_ENABLED
		FORCEINLINE virtual const char* Debug_GetName() const = 0;
		FORCEINLINE virtual std::string Debug_ToString() const { return Debug_GetName(); }
#endif

		FORCEINLINE bool IsDeferred() const { return m_bDefer; }
		
		// Utility

		/* Don't ever call this on stack allocated (non-deferred) events! */
		Shared<Event> AsShared();

		/* Creates an event that is called after the application loop is completed. */
		template<typename EventT, typename... Types>
		static std::enable_if_t<std::is_base_of_v<Event, EventT>, EventT*> CreateDeferredEvent(Types&&... args)
		{
			EventT* event = new EventT(args...);
			event->m_bDefer = true;
			return event;
		}

	protected:
		bool m_bHandled = false;
		bool m_bDefer = false;
	};

	using DeferredEventPtr = Event*;
}

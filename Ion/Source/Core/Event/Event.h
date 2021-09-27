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
		WindowChangeDisplayMode,

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

	enum EEventCategory : uint32
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

#define _SET_EVENT_TYPE(type) \
static EEventType GetStaticType() { return EEventType::##type; } \
virtual EEventType GetType() const override { return GetStaticType(); } \

#ifdef ION_LOG_ENABLED
	#define SET_EVENT_TYPE(type) _SET_EVENT_TYPE(type) \
	FORCEINLINE virtual const char* Debug_GetName() const override { return (#type"Event"); }
#else
	#define SET_EVENT_TYPE(type) _SET_EVENT_TYPE(type)
#endif

#define SET_EVENT_CATEGORY(category) \
	FORCEINLINE virtual uint32 GetCategoryFlags() const override { return category; }

#ifdef ION_LOG_ENABLED // Debug / Release
	#define SET_EVENT_TOSTRING_FORMAT(format) \
	FORCEINLINE virtual String Debug_ToString() const override \
	{ \
		std::stringstream ss; \
		ss << Debug_GetName() << ": " << format; \
		return ss.str(); \
	}
#else
	#define SET_EVENT_TOSTRING_FORMAT(format)
#endif

	// @TODO: This whole event system will need some kind of rework in the future.

	// Classes

	class ION_API Event
	{
		friend class EventDispatcher;
		friend class EventQueue;

	public:
		FORCEINLINE virtual EEventType GetType() const = 0;
		FORCEINLINE virtual uint32 GetCategoryFlags() const = 0;

		FORCEINLINE bool IsInCategory(EEventCategory category) const
		{
			return GetCategoryFlags() & (uint32)category;
		}

#ifdef ION_LOG_ENABLED
		FORCEINLINE virtual const char* Debug_GetName() const = 0;
		FORCEINLINE virtual String Debug_ToString() const { return Debug_GetName(); }
#endif

	protected:
		bool m_bHandled = false;
	};

	class ION_API DeferredEvent
	{
	public:
		DeferredEvent(const DeferredEvent& event)
			: m_EventPtr(event.m_EventPtr)
		{ }

		DeferredEvent(DeferredEvent&& event) noexcept
			: m_EventPtr(std::move(event.m_EventPtr))
		{ }

		template<typename T, typename TEnableIfT<TIsBaseOfV<Event, T>>* = nullptr, typename... ArgsT>
		FORCEINLINE static DeferredEvent Create(ArgsT&&... args)
		{
			T* event = new T(std::forward<ArgsT>(args)...);
			return DeferredEvent(event);
		}

		DeferredEvent operator=(const DeferredEvent& event)
		{
			m_EventPtr = event.m_EventPtr;
			return *this;
		}
		Event& operator->() const
		{
			return *m_EventPtr;
		}
		Event& operator*() const
		{
			return *m_EventPtr;
		}

	private:
		DeferredEvent(Event* eventPtr)
		{
			m_EventPtr = MakeShareable(eventPtr);
		}

	private:
		TShared<Event> m_EventPtr;
	};
}

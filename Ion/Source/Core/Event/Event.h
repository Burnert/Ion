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

#if ION_DEBUG
#define SET_DEBUG_INFO_STRING(format, ...) \
{ \
	char str[256]; \
	memset(str, 0, sizeof(str)); \
	sprintf_s(str, format, __VA_ARGS__); \
	m_DebugInfo = str; \
}
#else
#define SET_DEBUG_INFO_STRING(format)
#endif

#define STATIC_EVENT_TYPE_GETTER(type) static inline EEventType _GetType() { return type; }

	class ION_API Event
	{
		friend class EventDispatcher;

		//template<void(const Event&)>
		//friend class EventQueue;

	public:
		FORCEINLINE EEventType GetType() const { return m_Type; }
		FORCEINLINE uint32 GetCategoryFlags() const { return m_CategoryFlags; }
		FORCEINLINE bool IsInCategory(EEventCategory category) const { return m_CategoryFlags & (uint32)category; }

	protected:
		EEventType m_Type;
		uint32 m_CategoryFlags;

#if ION_DEBUG
		String m_DebugInfo;
#endif
	};
}

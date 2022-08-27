#pragma once

namespace Ion
{
	enum class EEventType : uint8
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

		// Mouse

		MouseMoved,
		MouseScrolled,
		MouseButtonPressed,
		MouseButtonReleased,
		MouseDoubleClick,

		// Raw Input Mouse

		RawInputMouseMoved,
		RawInputMouseScrolled,
		RawInputMouseButtonPressed,
		RawInputMouseButtonReleased,

		_Count
	};

	namespace EEventCategory
	{
		enum Type : uint8
		{
			None        = 0,
			Application = 1 << 0,
			Window      = 1 << 1,
			Input       = 1 << 2,
			Keyboard    = 1 << 3,
			Mouse       = 1 << 4,
			MouseButton = 1 << 5,
			RawInput    = 1 << 6,
		};
	}

#pragma region Event Base

#if ION_DEBUG
#define SET_DEBUG_STRING(fmtString, ...) m_Debug = fmt::format(fmtString, __VA_ARGS__)
#else
#define SET_DEBUG_STRING(format, ...)
#endif

#define STATIC_EVENT_CLASS_TYPE_GETTER(type) FORCEINLINE static EEventType GetClassType() { return type; }

	struct Event
	{
#if ION_DEBUG
	protected:
		String m_Debug;
	public:
#endif
		const uint32 CategoryFlags;
		const EEventType Type;

		Event(EEventType type, uint32 category);

		bool IsInCategory(EEventCategory::Type category) const;
	};

	inline Event::Event(EEventType type, uint32 category) :
		Type(type),
		CategoryFlags(category)
	{
	}

	FORCEINLINE bool Event::IsInCategory(EEventCategory::Type category) const
	{
		return CategoryFlags & (uint32)category;
	}

#pragma endregion

#pragma region Window Events

#if ION_DEBUG
#define _EVENT_DEBUG_NAME(name) [&](auto& str) -> String { if constexpr (TIsConvertibleV<decltype(str), String>) return str; else return StringConverter::WStringToString(str); }(name)
#else
#define _EVENT_DEBUG_NAME(name) EmptyString
#endif
/**
 * @brief Used to pass a debug name to a WindowEvent.
 */
#define EVENT_DEBUG_NAME(name) _EVENT_DEBUG_NAME(name)

	struct WindowEvent : Event
	{
		void* const WindowHandle;

	protected:
		WindowEvent(EEventType type, void* const windowHandle) :
			Event(type, EEventCategory::Application | EEventCategory::Window),
			WindowHandle(windowHandle)
		{
		}
	};

	struct WindowCloseEvent : WindowEvent
	{
		WindowCloseEvent(void* const windowHandle, const String& debugName = EmptyString) :
			WindowEvent(EEventType::WindowClose, windowHandle)
		{
			SET_DEBUG_STRING("WindowCloseEvent: {{ Name: \"{}\", Handle: {{{}}} }}", debugName, WindowHandle);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::WindowClose)
	};

	struct WindowResizeEvent : WindowEvent
	{
		const uint32 Width;
		const uint32 Height;

		WindowResizeEvent(void* const windowHandle, uint32 width, uint32 height, const String& debugName = EmptyString) :
			WindowEvent(EEventType::WindowResize, windowHandle),
			Width(width),
			Height(height)
		{
			SET_DEBUG_STRING("WindowResizeEvent: {{ Name: \"{}\", Handle: {{{}}}, Width: {}, Height: {} }}", debugName, WindowHandle, Width, Height);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::WindowResize)
	};

	struct WindowMovedEvent : WindowEvent
	{
		const int32 X;
		const int32 Y;

		WindowMovedEvent(void* const windowHandle, int32 x, int32 y, const String& debugName = EmptyString) :
			WindowEvent(EEventType::WindowMoved, windowHandle),
			X(x),
			Y(y)
		{
			SET_DEBUG_STRING("WindowMovedEvent: {{ Name: \"{}\", Handle: {{{}}}, X: {}, Y: {} }}", debugName, WindowHandle, X, Y);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::WindowMoved)
	};

	struct WindowFocusEvent : WindowEvent
	{
		WindowFocusEvent(void* const windowHandle, const String& debugName = EmptyString) :
			WindowEvent(EEventType::WindowFocus, windowHandle)
		{
			SET_DEBUG_STRING("WindowFocusEvent: {{ Name: \"{}\", Handle: {{{}}} }}", debugName, WindowHandle);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::WindowFocus);
	};

	struct WindowLostFocusEvent : WindowEvent
	{
		WindowLostFocusEvent(void* const windowHandle, const String& debugName = EmptyString) :
			WindowEvent(EEventType::WindowLostFocus, windowHandle)
		{
			SET_DEBUG_STRING("WindowLostFocusEvent: {{ Name: \"{}\", Handle: {{{}}} }}", debugName, WindowHandle);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::WindowLostFocus);
	};

	enum class EDisplayMode : uint8;

	struct WindowChangeDisplayModeEvent : WindowEvent
	{
		const EDisplayMode DisplayMode;
		const uint32 Width;
		const uint32 Height;

		WindowChangeDisplayModeEvent(void* const windowHandle, EDisplayMode displayMode, uint32 width, uint32 height, const String& debugName = EmptyString) :
			WindowEvent(EEventType::WindowChangeDisplayMode, windowHandle),
			DisplayMode(displayMode),
			Width(width),
			Height(height)
		{
			SET_DEBUG_STRING("WindowChangeDisplayModeEvent: {{ Name: \"{}\", Handle: {{{}}}, Mode: {}, Width: {}, Height: {} }}", debugName, WindowHandle, (uint8)DisplayMode, Width, Height);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::WindowChangeDisplayMode);
	};

#pragma endregion

#pragma region Input Events

#pragma region Keyboard

	struct KeyboardEvent : Event
	{
		const uint32 KeyCode;
		const uint32 ActualKeyCode;

	protected:
		KeyboardEvent(EEventType type, uint32 keyCode, uint32 actualKeyCode) :
			Event(type, EEventCategory::Input | EEventCategory::Keyboard),
			KeyCode(keyCode),
			ActualKeyCode(actualKeyCode)
		{
		}
	};

	struct KeyPressedEvent : KeyboardEvent
	{
		KeyPressedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(EEventType::KeyPressed, keyCode, actualKeyCode)
		{
			SET_DEBUG_STRING("KeyPressedEvent: {{ KeyCode: {:#x}, ActualKeyCode: {:#x} }}", KeyCode, ActualKeyCode);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::KeyPressed);
	};

	struct KeyReleasedEvent : KeyboardEvent
	{
		KeyReleasedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(EEventType::KeyReleased, keyCode, actualKeyCode)
		{
			SET_DEBUG_STRING("KeyReleasedEvent: {{ KeyCode: {:#x}, ActualKeyCode: {:#x} }}", KeyCode, ActualKeyCode);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::KeyReleased);
	};

	struct KeyRepeatedEvent : KeyboardEvent
	{
		KeyRepeatedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(EEventType::KeyRepeated, keyCode, actualKeyCode)
		{
			SET_DEBUG_STRING("KeyRepeatedEvent: {{ KeyCode: {:#x}, ActualKeyCode: {:#x} }}", KeyCode, ActualKeyCode);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::KeyRepeated);
	};

#pragma endregion

#pragma region Mouse

	struct MouseMovedEvent : Event
	{
		const float X;
		const float Y;
		const int32 ScreenX;
		const int32 ScreenY;

		MouseMovedEvent(float x, float y, int32 ssx, int32 ssy) :
			Event(EEventType::MouseMoved, EEventCategory::Input | EEventCategory::Mouse),
			X(x),
			Y(y),
			ScreenX(ssx),
			ScreenY(ssy)
		{
			SET_DEBUG_STRING("MouseMovedEvent: {{ X: {:.2}, Y: {:.2}, SSX: {}, SSY: {} }}", X, Y, ScreenX, ScreenY);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::MouseMoved);
	};

	struct MouseScrolledEvent : Event
	{
		const float Offset;

		MouseScrolledEvent(float offset) :
			Event(EEventType::MouseScrolled, EEventCategory::Input | EEventCategory::Mouse),
			Offset(offset)
		{
			SET_DEBUG_STRING("MouseScrolledEvent: {{ Offset: {:.2} }}", Offset);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::MouseScrolled);
	};

	// Mouse Button Events

	struct MouseButtonEvent : Event
	{
		const uint32 Button;

	protected:
		MouseButtonEvent(EEventType type, uint32 button) :
			Event(type, EEventCategory::Input | EEventCategory::Mouse | EEventCategory::MouseButton),
			Button(button)
		{
		}
	};

	struct MouseButtonPressedEvent : MouseButtonEvent
	{
		MouseButtonPressedEvent(uint32 button) :
			MouseButtonEvent(EEventType::MouseButtonPressed, button)
		{
			SET_DEBUG_STRING("MouseButtonPressedEvent: {{ Button: {} }}", Button);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::MouseButtonPressed);
	};

	struct MouseDoubleClickEvent : MouseButtonEvent
	{
		MouseDoubleClickEvent(uint32 button) :
			MouseButtonEvent(EEventType::MouseDoubleClick, button)
		{
			SET_DEBUG_STRING("MouseDoubleClickEvent: {{ Button: {} }}", Button);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::MouseDoubleClick);
	};

	struct MouseButtonReleasedEvent : MouseButtonEvent
	{
		MouseButtonReleasedEvent(uint32 button) :
			MouseButtonEvent(EEventType::MouseButtonReleased, button)
		{
			SET_DEBUG_STRING("MouseButtonReleasedEvent: {{ Button: {} }}", Button);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::MouseButtonReleased);
	};

#pragma endregion

#pragma region Raw Input

	// Mouse events

	struct RawInputMouseMovedEvent : Event
	{
		const float X;
		const float Y;

		RawInputMouseMovedEvent(float x, float y) :
			Event(EEventType::RawInputMouseMoved, EEventCategory::RawInput | EEventCategory::Input | EEventCategory::Mouse),
			X(x),
			Y(y)
		{
			SET_DEBUG_STRING("RawInputMouseMovedEvent: {{ X: {:.2}, Y: {:.2} }}", X, Y);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::RawInputMouseMoved);
	};

	struct RawInputMouseScrolledEvent : Event
	{
		const float Offset;

		RawInputMouseScrolledEvent(float offset) :
			Event(EEventType::RawInputMouseScrolled, EEventCategory::RawInput | EEventCategory::Input | EEventCategory::Mouse),
			Offset(offset)
		{
			SET_DEBUG_STRING("RawInputMouseScrolledEvent: {{ Offset: {:.2} }}", Offset);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::RawInputMouseScrolled);
	};

	// Mouse Button Events

	struct RawInputMouseButtonEvent : Event
	{
		const uint32 Button;

	protected:
		RawInputMouseButtonEvent(EEventType type, uint32 button) :
			Event(type, EEventCategory::RawInput | EEventCategory::Input | EEventCategory::Mouse | EEventCategory::MouseButton),
			Button(button)
		{
		}
	};

	struct RawInputMouseButtonPressedEvent : RawInputMouseButtonEvent
	{
		RawInputMouseButtonPressedEvent(uint32 button) :
			RawInputMouseButtonEvent(EEventType::RawInputMouseButtonPressed, button)
		{
			SET_DEBUG_STRING("RawInputMouseButtonPressedEvent: {{ Button: {} }}", Button);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::RawInputMouseButtonPressed);
	};

	struct RawInputMouseButtonReleasedEvent : RawInputMouseButtonEvent
	{
		RawInputMouseButtonReleasedEvent(uint32 button) :
			RawInputMouseButtonEvent(EEventType::RawInputMouseButtonReleased, button)
		{
			SET_DEBUG_STRING("RawInputMouseButtonReleasedEvent: {{ Button: {} }}", Button);
		}

		STATIC_EVENT_CLASS_TYPE_GETTER(EEventType::RawInputMouseButtonReleased);
	};

#pragma endregion

#pragma endregion
}

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

#define EVENT_CLASS_BODY(type) \
virtual std::unique_ptr<const Event> Defer() const override { return std::make_unique<TRemoveRef<decltype(*this)>>(*this); } \
FORCEINLINE static EEventType GetClassType() { return type; }

	struct NOVTABLE Event
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

		virtual std::unique_ptr<const Event> Defer() const = 0;
	};

	inline Event::Event(EEventType type, uint32 category) :
		CategoryFlags(category),
		Type(type)
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

	struct WindowCloseEvent final : WindowEvent
	{
		WindowCloseEvent(void* const windowHandle, const String& debugName = EmptyString) :
			WindowEvent(EEventType::WindowClose, windowHandle)
		{
			SET_DEBUG_STRING("WindowCloseEvent: {{ Name: \"{}\", Handle: {{{}}} }}", debugName, WindowHandle);
		}

		EVENT_CLASS_BODY(EEventType::WindowClose)
	};

	struct WindowResizeEvent final : WindowEvent
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

		EVENT_CLASS_BODY(EEventType::WindowResize)
	};

	struct WindowMovedEvent final : WindowEvent
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

		EVENT_CLASS_BODY(EEventType::WindowMoved)
	};

	struct WindowFocusEvent final : WindowEvent
	{
		WindowFocusEvent(void* const windowHandle, const String& debugName = EmptyString) :
			WindowEvent(EEventType::WindowFocus, windowHandle)
		{
			SET_DEBUG_STRING("WindowFocusEvent: {{ Name: \"{}\", Handle: {{{}}} }}", debugName, WindowHandle);
		}

		EVENT_CLASS_BODY(EEventType::WindowFocus);
	};

	struct WindowLostFocusEvent final : WindowEvent
	{
		WindowLostFocusEvent(void* const windowHandle, const String& debugName = EmptyString) :
			WindowEvent(EEventType::WindowLostFocus, windowHandle)
		{
			SET_DEBUG_STRING("WindowLostFocusEvent: {{ Name: \"{}\", Handle: {{{}}} }}", debugName, WindowHandle);
		}

		EVENT_CLASS_BODY(EEventType::WindowLostFocus);
	};

	enum class EDisplayMode : uint8;

	struct WindowChangeDisplayModeEvent final : WindowEvent
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

		EVENT_CLASS_BODY(EEventType::WindowChangeDisplayMode);
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

	struct KeyPressedEvent final : KeyboardEvent
	{
		KeyPressedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(EEventType::KeyPressed, keyCode, actualKeyCode)
		{
			SET_DEBUG_STRING("KeyPressedEvent: {{ KeyCode: {:#x}, ActualKeyCode: {:#x} }}", KeyCode, ActualKeyCode);
		}

		EVENT_CLASS_BODY(EEventType::KeyPressed);
	};

	struct KeyReleasedEvent final : KeyboardEvent
	{
		KeyReleasedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(EEventType::KeyReleased, keyCode, actualKeyCode)
		{
			SET_DEBUG_STRING("KeyReleasedEvent: {{ KeyCode: {:#x}, ActualKeyCode: {:#x} }}", KeyCode, ActualKeyCode);
		}

		EVENT_CLASS_BODY(EEventType::KeyReleased);
	};

	struct KeyRepeatedEvent final : KeyboardEvent
	{
		KeyRepeatedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(EEventType::KeyRepeated, keyCode, actualKeyCode)
		{
			SET_DEBUG_STRING("KeyRepeatedEvent: {{ KeyCode: {:#x}, ActualKeyCode: {:#x} }}", KeyCode, ActualKeyCode);
		}

		EVENT_CLASS_BODY(EEventType::KeyRepeated);
	};

#pragma endregion

#pragma region Mouse

	struct MouseMovedEvent final : Event
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

		EVENT_CLASS_BODY(EEventType::MouseMoved);
	};

	struct MouseScrolledEvent final : Event
	{
		const float Offset;

		MouseScrolledEvent(float offset) :
			Event(EEventType::MouseScrolled, EEventCategory::Input | EEventCategory::Mouse),
			Offset(offset)
		{
			SET_DEBUG_STRING("MouseScrolledEvent: {{ Offset: {:.2} }}", Offset);
		}

		EVENT_CLASS_BODY(EEventType::MouseScrolled);
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

	struct MouseButtonPressedEvent final : MouseButtonEvent
	{
		MouseButtonPressedEvent(uint32 button) :
			MouseButtonEvent(EEventType::MouseButtonPressed, button)
		{
			SET_DEBUG_STRING("MouseButtonPressedEvent: {{ Button: {} }}", Button);
		}

		EVENT_CLASS_BODY(EEventType::MouseButtonPressed);
	};

	struct MouseDoubleClickEvent final : MouseButtonEvent
	{
		MouseDoubleClickEvent(uint32 button) :
			MouseButtonEvent(EEventType::MouseDoubleClick, button)
		{
			SET_DEBUG_STRING("MouseDoubleClickEvent: {{ Button: {} }}", Button);
		}

		EVENT_CLASS_BODY(EEventType::MouseDoubleClick);
	};

	struct MouseButtonReleasedEvent final : MouseButtonEvent
	{
		MouseButtonReleasedEvent(uint32 button) :
			MouseButtonEvent(EEventType::MouseButtonReleased, button)
		{
			SET_DEBUG_STRING("MouseButtonReleasedEvent: {{ Button: {} }}", Button);
		}

		EVENT_CLASS_BODY(EEventType::MouseButtonReleased);
	};

#pragma endregion

#pragma region Raw Input

	// Mouse events

	struct RawInputMouseMovedEvent final : Event
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

		EVENT_CLASS_BODY(EEventType::RawInputMouseMoved);
	};

	struct RawInputMouseScrolledEvent final : Event
	{
		const float Offset;

		RawInputMouseScrolledEvent(float offset) :
			Event(EEventType::RawInputMouseScrolled, EEventCategory::RawInput | EEventCategory::Input | EEventCategory::Mouse),
			Offset(offset)
		{
			SET_DEBUG_STRING("RawInputMouseScrolledEvent: {{ Offset: {:.2} }}", Offset);
		}

		EVENT_CLASS_BODY(EEventType::RawInputMouseScrolled);
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

	struct RawInputMouseButtonPressedEvent final : RawInputMouseButtonEvent
	{
		RawInputMouseButtonPressedEvent(uint32 button) :
			RawInputMouseButtonEvent(EEventType::RawInputMouseButtonPressed, button)
		{
			SET_DEBUG_STRING("RawInputMouseButtonPressedEvent: {{ Button: {} }}", Button);
		}

		EVENT_CLASS_BODY(EEventType::RawInputMouseButtonPressed);
	};

	struct RawInputMouseButtonReleasedEvent final : RawInputMouseButtonEvent
	{
		RawInputMouseButtonReleasedEvent(uint32 button) :
			RawInputMouseButtonEvent(EEventType::RawInputMouseButtonReleased, button)
		{
			SET_DEBUG_STRING("RawInputMouseButtonReleasedEvent: {{ Button: {} }}", Button);
		}

		EVENT_CLASS_BODY(EEventType::RawInputMouseButtonReleased);
	};

#pragma endregion

#pragma endregion
}

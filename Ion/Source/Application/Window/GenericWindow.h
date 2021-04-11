#pragma once

#include "Core/CoreTypes.h"
#include "Core/CoreApi.h"

#include "Core/Event/Event.h"

namespace Ion
{
	struct WindowDimensions
	{
		int Width;
		int Height;

		inline float GetAspectRatio()
		{
			return (float)Width / (float)Height;
		}
	};

	enum class EDisplayMode : ubyte
	{
		Windowed,
		BorderlessWindow,
		FullScreen,
	};

	class ION_API GenericWindow
	{
	public:
		using EventPtr = TShared<Event>;
		using EventCallback = std::function<void(Event&)>;
		using DeferredEventCallback = std::function<void(DeferredEvent&)>;

		virtual ~GenericWindow() { }

		virtual bool Initialize();
		virtual bool Initialize(const TShared<GenericWindow>& parentWindow);

		virtual void Show() { }
		virtual void Hide() { }

		virtual void SetTitle(const std::wstring& title) { }

		virtual void SetEnabled(bool bEnabled) { }

		virtual void Resize() { }
		virtual WindowDimensions GetDimensions() const;

		virtual void EnableFullScreen(bool bFullscreen) { }
		virtual bool IsFullScreenEnabled() const;

		virtual bool IsInFocus() const;

		/* Clips the cursor to the client area.
		   The cursor will not move outside the window. */
		virtual void ClipCursor() { }
		/* Locks the cursor in the specified point (relative to window client area).
		   The cursor will not move at all. */
		virtual void LockCursor(IVector2 position) { }
		/* Locks the cursor in the current position.
		   The cursor will not move at all. */
		virtual void LockCursor() { }
		virtual void UnlockCursor() { }
		FORCEINLINE bool IsCursorLocked() const { return m_bCursorLocked; }

		virtual void ShowCursor(bool bShow) { }

		virtual void* GetNativeHandle() const;

		virtual void MakeRenderingContextCurrent() { }

		virtual void SwapBuffers() { }

		FVector2 GetCenterPosition() const;

		void SetEventCallback(EventCallback callback);
		void SetDeferredEventCallback(DeferredEventCallback callback);

	public:
		// Implemented per platform.
		static TShared<GenericWindow> Create();

	protected:
		// Protected constructor: Only shared_ptrs of this class can be made.
		GenericWindow();

		// Function that gets called every time an event occurs.
		EventCallback m_EventCallback;
		DeferredEventCallback m_DeferredEventCallback;

		bool m_bCursorLocked;
		bool m_bCursorShown;
	};

	
}


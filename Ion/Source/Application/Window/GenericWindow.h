#pragma once

#include "Application/Event/Event.h"

namespace Ion
{
	struct WindowDimensions
	{
		int32 Width;
		int32 Height;

		inline float GetAspectRatio()
		{
			return (Height != 0.0f) ? ((float)Width / (float)Height) : 1.0f;
		}

		inline operator UVector2() const
		{
			return UVector2(Width, Height);
		}
	};

	enum class EDisplayMode : uint8
	{
		Windowed,
		BorderlessWindow,
		FullScreen,
	};

	class ION_API GenericWindow
	{
	public:
		virtual ~GenericWindow() { }

		virtual bool Initialize();
		virtual bool Initialize(const std::shared_ptr<GenericWindow>& parentWindow);

		virtual void Show() { }
		virtual void Hide() { }
		virtual void Maximize() { }

		virtual void SetTitle(const WString& title) { }

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
		virtual void LockCursor(bool bLock) { }
		virtual void UnlockCursor() { }
		FORCEINLINE bool IsCursorLocked() const { return m_bCursorLocked; }

		virtual void ShowCursor(bool bShow) { }

		virtual void* GetNativeHandle() const;

		virtual void MakeRenderingContextCurrent() { }

		virtual void SwapBuffers() { }

		Vector2 GetCenterPosition() const;

		const std::shared_ptr<RHITexture>& GetWindowColorTexture() const;
		const std::shared_ptr<RHITexture>& GetWindowDepthStencilTexture() const;

	public:
		// Implemented per platform.
		static std::shared_ptr<GenericWindow> Create();

	protected:
		// Protected constructor: Only shared_ptrs of this class can be made.
		GenericWindow();

	private:
		std::shared_ptr<RHITexture> m_WindowColorTexture;
		std::shared_ptr<RHITexture> m_WindowDepthStencilTexture;

	protected:
		bool m_bCursorLocked;
		bool m_bCursorShown;

		friend class DX11;
		friend class DX10;
	};

	inline const std::shared_ptr<RHITexture>& GenericWindow::GetWindowColorTexture() const
	{
		return m_WindowColorTexture;
	}

	inline const std::shared_ptr<RHITexture>& GenericWindow::GetWindowDepthStencilTexture() const
	{
		return m_WindowDepthStencilTexture;
	}
}

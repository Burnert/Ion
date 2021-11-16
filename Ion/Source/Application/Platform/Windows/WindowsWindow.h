#pragma once

#include "Application/Window/GenericWindow.h"

namespace Ion
{
	struct SWindowDataBeforeFullScreen
	{
		WINDOWPLACEMENT WindowPlacement;
		DWORD Style;
		bool bMaximized;
	};

	class ION_API WindowsWindow : public GenericWindow
	{
		friend class WindowsApplication;
	public:
		static TShared<WindowsWindow> Create();

		// GenericWindow:

		virtual ~WindowsWindow();

		virtual bool Initialize() override;
		virtual bool Initialize(const TShared<GenericWindow>& parentWindow) override;

		virtual void Show() override;
		virtual void Hide() override;

		virtual void SetTitle(const WString& title) override;

		virtual void SetEnabled(bool bEnabled) override;

		virtual void Resize() override;
		virtual WindowDimensions GetDimensions() const override;

		virtual void EnableFullScreen(bool bFullscreen) override;
		virtual bool IsFullScreenEnabled() const override;

		virtual bool IsInFocus() const;

		virtual void ClipCursor() override;
		virtual void LockCursor(IVector2 position) override;
		virtual void LockCursor() override;
		virtual void UnlockCursor() override;

		virtual void ShowCursor(bool bShow) override;

		virtual void* GetNativeHandle() const override;

		virtual void MakeRenderingContextCurrent() override;

		virtual void SwapBuffers() override;

		// End of GenericWindow

		HDC GetDeviceContext() const;
		HGLRC GetRenderingContext() const;
		FORCEINLINE HWND GetWindowHandle() const { return m_WindowHandle; }

		static bool RegisterWindowClass(HINSTANCE hInstance, LPCWSTR className);

		FORCEINLINE static HGLRC GetCurrentRenderingContext() { return wglGetCurrentContext(); }

	protected:
		HGLRC CreateRenderingContext(HDC deviceContext, HGLRC parentContext = nullptr);
		void DeleteRenderingContext();

	protected:
		// Protected constructor: Only shared_ptrs of this class can be made.
		WindowsWindow();

		void PollEvents_Application();

		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		HWND  m_WindowHandle;
		HDC   m_DeviceContext;
		HGLRC m_RenderingContext;

		WString m_Title;
		bool m_bVisible = false;

		// Should be true after Initialize() has been called one time.
		static bool s_bRegistered;

		static bool m_bBothShiftsPressed;

		bool m_bFullScreenMode = false;
		SWindowDataBeforeFullScreen m_WindowBeforeFullScreen { };

		static MouseButton MouseButtonFromMessage(UINT uMsg, WPARAM wParam);

		friend class OpenGLWindows;
	};
}

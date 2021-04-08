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

		virtual void Show() override;
		virtual void Hide() override;

		virtual void SetTitle(const wchar* title) override;

		virtual void SetEnabled(bool bEnabled) override;

		virtual void Resize() override;
		virtual WindowDimensions GetDimensions() const override;

		virtual void EnableFullScreen(bool bFullscreen) override;
		virtual bool IsFullScreenEnabled() const override;

		virtual bool IsInFocus() const;

		virtual void ClipCursor(bool bClip) const override;
		virtual void LockCursor(IVector2 position) const override;
		virtual void UnlockCursor() const override;

		virtual void ShowCursor(bool bShow) const override;

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
		HGLRC CreateRenderingContext(HDC deviceContext);
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

		std::wstring m_Title;
		bool m_bVisible = false;

		// Should be true after Initialize() has been called.
		bool m_bRegistered = false;

		static bool m_bBothShiftsPressed;

		bool m_bFullScreenMode = false;
		SWindowDataBeforeFullScreen m_WindowBeforeFullScreen { };

		static MouseButton MouseButtonFromMessage(UINT uMsg, WPARAM wParam);
	};
}

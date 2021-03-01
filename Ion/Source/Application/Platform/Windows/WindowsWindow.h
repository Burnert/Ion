#pragma once

#include "Application/Window/GenericWindow.h"

namespace Ion
{
	class ION_API WindowsWindow : public GenericWindow
	{
		friend class WindowsApplication;
	public:
		static Shared<WindowsWindow> Create();

		// GenericWindow:

		virtual ~WindowsWindow();

		virtual bool Initialize() override;

		virtual void Show() override;
		virtual void Hide() override;

		virtual void SetTitle(const WCStr title) override;

		virtual void SetEnabled(bool bEnabled);

		virtual void Resize() override;
		virtual WindowDimensions GetDimensions() const override;

		virtual void MakeRenderingContextCurrent() override;

		// End of GenericWindow

		HDC GetDeviceContext() const;
		FORCEINLINE HWND GetWindowHandle() const { return m_HWnd; }

		bool RegisterWindowClass(HINSTANCE hInstance, LPCWSTR className);

		FORCEINLINE static HGLRC GetCurrentRenderingContext() { return wglGetCurrentContext(); }

	protected:
		HGLRC CreateRenderingContext(HDC deviceContext);

	protected:
		// Protected constructor: Only shared_ptrs of this class can be made.
		WindowsWindow();

		void PollEvents_Application();

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		HWND m_HWnd;
		std::wstring m_Title;
		bool m_bVisible = false;

		// Should be true after Initialize() has been called.
		bool m_bRegistered = false;

		static bool m_bBothShiftsPressed;

		static MouseButton MouseButtonFromMessage(UINT uMsg, WPARAM wParam);
	};
}

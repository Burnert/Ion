#pragma once

#include "Application/Window/GenericWindow.h"

namespace Ion
{
	class ION_API WindowsWindow : public GenericWindow
	{
	public:
		static std::shared_ptr<WindowsWindow> Create();

		virtual ~WindowsWindow();

		bool RegisterWindowClass(HINSTANCE hInstance, LPCWSTR className);

		virtual bool Initialize() override;

		virtual void Show() override;
		virtual void Hide() override;

		virtual void SetTitle(const WCStr title) override;

		virtual void SetEnabled(bool bEnabled);

		virtual void Resize() override;
		virtual bool GetDimensions(int& width, int& height) const override;

		FORCEINLINE HWND GetHWnd() const { return m_HWnd; }
		HDC GetHDC() const;

	protected:
		// Protected constructor: Only shared_ptrs of this class can be made.
		WindowsWindow();

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		HWND m_HWnd;
		std::wstring m_Title;
		bool m_bVisible = false;

		// Should be true after Initialize() has been called.
		bool m_bRegistered = false;
	};
}

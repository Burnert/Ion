#pragma once

#include "Application/Window/GenericWindow.h"

namespace Ion
{
	class WindowsWindow : public GenericWindow
	{
	public:
		static std::shared_ptr<WindowsWindow> Create();

		virtual ~WindowsWindow();

		virtual void Initialize() override;

		virtual void SetTitle(WCStr title) override;

		virtual void Resize() override;
		virtual bool GetDimensions(int& width, int& height) const override;

		FORCEINLINE HWND GetHWnd() const { return m_HWnd; }
		FORCEINLINE HDC GetHDC() const { return m_HDC; }

	protected:
		// Protected constructor: Only shared_ptrs of this class can be made.
		WindowsWindow();

	private:
		HWND m_HWnd;
		HDC m_HDC;
	};
}

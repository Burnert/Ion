#pragma once

#include "Application/Window/GenericWindow.h"

namespace Ion
{
	class WindowsWindow : public GenericWindow
	{
	public:
		WindowsWindow();
		virtual ~WindowsWindow();

		virtual void SetTitle(WCStr title) override;

		virtual void Resize() override;
		virtual bool GetDimensions(int& width, int& height) const override;
	};
}

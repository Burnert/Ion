#pragma once

#include "Core/CoreApi.h"
#include "Application/Application.h"
#include "Core/Input/Input.h"

class WindowsInputManager;
class WindowsWindow;

namespace Ion
{
	class ION_API WindowsApplication : public Application
	{
	public:
		static WindowsApplication* Get();

		/* Called by the Entry Point */
		void InitWindows(HINSTANCE hInstance);

		FORCEINLINE static HINSTANCE GetHInstance() { return m_HInstance; }

	protected:
		// Tagged as final so it cannot be overriden in the client

		virtual void PollEvents() final override;
		virtual void Update(float DeltaTime) final override;
		virtual void Render() final override;
		virtual void HandleEvent(Event& event) final override;
		virtual void DispatchEvent(Event& event) final override;

		// End of final overrides

	private:
		virtual float CalculateFrameTime() override;

	private:
		static HINSTANCE m_HInstance;

		static float s_PerformanceFrequency;
		static float s_LastFrameTime;
	};
}

// Platform specific type of the Application class
using IonApplication = Ion::WindowsApplication;

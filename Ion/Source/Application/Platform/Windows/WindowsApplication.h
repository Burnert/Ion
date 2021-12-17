#pragma once

#include "Core/CoreApi.h"
#include "Application/Application.h"
#include "Core/Input/Input.h"

class WindowsInputManager;
class WindowsWindow;

struct ImDrawData;
struct ImGuiViewport;

namespace Ion
{
	class ION_API WindowsApplication : public Application
	{
		friend class Application;
	public:
		static WindowsApplication* Get();

		/* Called by the Entry Point */
		virtual void Start();

		void InitWindows(HINSTANCE hInstance);

		FORCEINLINE static HINSTANCE GetHInstance() { return m_HInstance; }

		static float GetPerformanceFrequency() { return s_PerformanceFrequency; }

	protected:
		// Tagged as final so it cannot be overriden in the client

		virtual void PollEvents() final override;
		virtual void Update(float DeltaTime) final override;
		virtual void Render() final override;

		// End of final overrides

		virtual void OnWindowCloseEvent_Internal(const WindowCloseEvent& event) override;

	private:
		virtual void InitImGuiBackend(const TShared<GenericWindow>& window) const override;
		virtual void ImGuiNewFramePlatform() const override;
		virtual void ImGuiRenderPlatform(ImDrawData* drawData) const override;
		virtual void ImGuiShutdownPlatform() const override;

	private:
		static HINSTANCE m_HInstance;

		static float s_PerformanceFrequency;
		//static LARGE_INTEGER s_liFirstFrameTime;
		//static float s_LastFrameTime;
	};
}

#pragma once

#include "Core/CoreApi.h"
#include "Core/Input/Input.h"
#include "Core/Platform/Windows/WindowsCore.h"
#include "Application/Application.h"

struct ImDrawData;
struct ImGuiViewport;

namespace Ion
{
	REGISTER_LOGGER(WindowsApplicationLogger, "Platform::Windows::Application");

	class ION_API WindowsApplication : public Application
	{
		friend class Application;
	public:
		static WindowsApplication* Get();

		/* Called by the Entry Point */
		virtual void Start();

		virtual void SetCursor(ECursorType cursor) override;
		virtual ECursorType GetCurrentCursor() const override;

		void InitWindows(HINSTANCE hInstance);

		FORCEINLINE static HINSTANCE GetHInstance() { return m_HInstance; }

		static float GetPerformanceFrequency() { return s_PerformanceFrequency; }

	protected:
		WindowsApplication(App* clientApp);

		// Tagged as final so it cannot be overriden in the client

		virtual void PollEvents() final override;
		virtual void Update(float DeltaTime) final override;
		virtual void Render() final override;

		// End of final overrides

		virtual void OnWindowCloseEvent_Internal(const WindowCloseEvent& event) override;

	private:
		void LoadCursors();
		bool UpdateMouseCursor();

		virtual void InitImGuiBackend(const TShared<GenericWindow>& window) const override;
		virtual void ImGuiNewFramePlatform() const override;
		virtual void ImGuiRenderPlatform(ImDrawData* drawData) const override;
		virtual void ImGuiShutdownPlatform() const override;

	private:
		static HINSTANCE m_HInstance;

		static float s_PerformanceFrequency;
		//static LARGE_INTEGER s_liFirstFrameTime;
		//static float s_LastFrameTime;

		HCURSOR m_CursorHandles[(size_t)ECursorType::_Count];
		int32 m_CurrentCursor;
		int32 m_RequestedCursor;

		friend Application* InstantiateApplication();
		friend class WindowsWindow;
	};
}

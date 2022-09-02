#pragma once

#include "Core/Platform/Windows.h"

#include "Application/Application.h"
#include "Application/Input/Input.h"

struct ImDrawData;
struct ImGuiViewport;

namespace Ion
{
	REGISTER_LOGGER(WindowsApplicationLogger, "Platform::Windows::Application");

	class ION_API WindowsApplication : public Application
	{
	public:
		static void PostEvent(const Event& e);
		static void PostDeferredEvent(const Event& e);
		static LRESULT CALLBACK WindowEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		WindowsApplication();

		static WindowsApplication* Get();

		virtual void SetCursor(ECursorType cursor) override;
		virtual ECursorType GetCurrentCursor() const override;

		static HINSTANCE GetHInstance();

		static float GetPerformanceFrequency();

	protected:
		virtual void PlatformInit() override;
		virtual void PlatformShutdown() override;

		virtual void RegisterRawInputDevices() override;

		virtual void PollEvents() override;
		virtual void Update(float DeltaTime) override;
		virtual void Render() override;

		virtual void OnWindowCloseEvent(const WindowCloseEvent& e) override;

	private:
		void LoadCursors();
		bool UpdateMouseCursor();

		void EventFixes();

		virtual void InitImGuiBackend(const std::shared_ptr<GenericWindow>& window) const override;
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

		friend class WindowsWindow;
		friend class Application;
	};

	FORCEINLINE HINSTANCE WindowsApplication::GetHInstance()
	{
		return m_HInstance;
	}

	FORCEINLINE float WindowsApplication::GetPerformanceFrequency()
	{
		return s_PerformanceFrequency;
	}
}

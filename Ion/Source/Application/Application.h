#pragma once

#include "Core.h"
#include "Application/Input/Input.h"
#include "Application/Event/Event.h"
#include "Application/Event/EventDispatcher.h"
#include "Application/Event/EventQueue.h"
#include "Application/Layer/LayerStack.h"
#include "Application/Window/GenericWindow.h"
#include "Application/EnginePath.h"

struct ImDrawData;
struct ImFont;

namespace Ion
{
	// Global Engine Application pointer (never null)
	extern class Application* const g_pEngineApplication;

	// Global Client Application pointer (never null)
	extern class IApp* const g_pClientApplication;

	REGISTER_LOGGER(ApplicationLogger, "Application");

	class Renderer;

	class RHIShader;
	class IndexBuffer;

	enum class ECursorType : int8
	{
		NoChange = -1, // Don't change the cursor
		Arrow = 0,
		Help,
		Cross,
		TextEdit,
		Unavailable,
		UpArrow,
		ResizeNS,
		ResizeWE,
		ResizeNWSE,
		ResizeNESW,
		Move,
		Hand,
		Grab,
		GrabClosed,
		_Count
	};

	struct EngineFonts
	{
		ImFont* System_14;
		ImFont* Roboto_14;
		ImFont* Roboto_16;
		ImFont* RobotoSlab_20;
		ImFont* Exo2_20;
		ImFont* Exo2_24;
	};

	class ION_API Application
	{
	public:
		virtual ~Application();

		/* Called by the Entry Point */
		virtual void Start();

		void Exit();

		void SetApplicationTitle(const WString& title);

		/* Sets the cursor for the current frame only. */
		virtual void SetCursor(ECursorType cursor);
		virtual ECursorType GetCurrentCursor() const;

		EngineFonts& GetEngineFonts();

		static Application* Get();

		static Renderer* GetRenderer();

		static const std::shared_ptr<GenericWindow>& GetWindow();
		static const std::shared_ptr<InputManager>& GetInputManager();
		static LayerStack* GetLayerStack();
		static float GetGlobalDeltaTime();

	protected:
		Application();

		void Init();
		void Run();
		void Shutdown();

		// Event system related functions

		friend void PostEvent(const Event& e);
		friend void PostDeferredEvent(const Event& e);

		void PostEvent(const Event& e);
		void PostDeferredEvent(const Event& e);

		void DispatchEvent(const Event& e);

		// Event functions

		virtual void OnWindowCloseEvent_Internal(const WindowCloseEvent& event); // Virtual because it's overriden in WindowsApplication
		void OnWindowResizeEvent_Internal(const WindowResizeEvent& event);
		void OnWindowChangeDisplayModeEvent_Internal(const WindowChangeDisplayModeEvent& event);
		void OnWindowLostFocusEvent_Internal(const WindowLostFocusEvent& event);
		void OnWindowFocusEvent_Internal(const WindowFocusEvent& event);

		void OnKeyPressedEvent_Internal(const KeyPressedEvent& event);
		void OnKeyReleasedEvent_Internal(const KeyReleasedEvent& event);
		void OnKeyRepeatedEvent_Internal(const KeyRepeatedEvent& event);

		/* Platform specific method for polling application events / messages. */
		virtual void PollEvents();

		virtual void Update(float DeltaTime);
		virtual void Render();

	protected:

		// To be overriden in client:

		/* Override this in the client if you want to use it.
		   Runs after the engine has been initialised. (before the PostInit stage) */
		virtual void OnInit() { }
		/* Override this in the client if you want to use it.
		   Runs every frame. */
		virtual void OnUpdate(float deltaTime) { }
		/* Override this in the client if you want to use it.
		   Runs every frame after the Update function. */
		virtual void OnRender() { }
		/* Override this in the client if you want to use it.
		   Runs after the engine has been shutdown and all the resources have been freed. */
		virtual void OnShutdown() { }
		/* Override this in the client if you want to use it.
		   Called when the application receives an event. */
		virtual void OnEvent(const Event& event) { }

		// End of overridables

		FORCEINLINE std::shared_ptr<GenericWindow> GetApplicationWindow() const { return m_Window; }

		static Application* s_Instance;

	private:
		void CallClientAppOnEvent(const Event& event);
		float CalculateFrameTime(); // Implemented per platform

		void SetRenderTargetToMainWindow();

		void SetupWindowTitle();
		void UpdateWindowTitle(float deltaTime);

		// @TODO: Move ImGui stuff to some generic Platform class

		void InitImGui() const;
		void ShutdownImGui() const;

		void SetImGuiCursor(int32 cursor);

		void LoadFonts();

	protected:
		virtual void InitImGuiBackend(const std::shared_ptr<GenericWindow>& window) const { }
		virtual void ImGuiNewFramePlatform() const { }
		virtual void ImGuiRenderPlatform(ImDrawData* drawData) const { }
		virtual void ImGuiShutdownPlatform() const { }

	private:
		std::shared_ptr<GenericWindow> m_Window;
		std::shared_ptr<InputManager> m_InputManager;

		TEventDispatcher<Application> m_EventDispatcher;
		EventQueue m_EventQueue;

		std::unique_ptr<LayerStack> m_LayerStack;

		std::thread::id m_MainThreadId;

		//WString m_BaseWindowTitle;
		WString m_ApplicationTitle;

		EngineFonts m_Fonts;

		float m_GlobalDeltaTime;

		bool m_bInFocus;
		bool m_bRunning;

		friend GenericWindow;
		friend int32 MainShared(int32 argc, tchar* argv[]);
		friend void ParseCommandLineArgs(int32 argc, tchar* argv[]);
	};

	FORCEINLINE Application* Application::Get()
	{
		return g_pEngineApplication;
	}

	inline EngineFonts& Application::GetEngineFonts()
	{
		return m_Fonts;
	}

	FORCEINLINE const std::shared_ptr<GenericWindow>& Application::GetWindow()
	{
		return Get()->m_Window;
	}

	FORCEINLINE const std::shared_ptr<InputManager>& Application::GetInputManager()
	{
		return Get()->m_InputManager;
	}

	FORCEINLINE LayerStack* Application::GetLayerStack()
	{
		return Get()->m_LayerStack.get();
	}

	FORCEINLINE float Application::GetGlobalDeltaTime()
	{
		return Get()->m_GlobalDeltaTime;
	}
}

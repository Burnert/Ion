#pragma once

#include "Core/Core.h"
#include "Core/Event/InputEvent.h"
#include "Core/Event/WindowEvent.h"
#include "Core/Layer/LayerStack.h"
//#include "Core/Asset/AssetManager.h"

#include "Application/Window/GenericWindow.h"
#include "Application/EnginePath.h"
#include "Core/Task/TaskFwd.h"

struct ImDrawData;
struct ImFont;

namespace Ion
{
	class App;

	template<void(const Event&)>
	class EventQueue;

	class InputManager;

	class Renderer;

	class Shader;
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
		friend GenericWindow;
	public:
		using EventPtr = TShared<Event>;

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
		template<typename T>
		static Application* Create(App* clientApp);

		static Renderer* GetRenderer();

		FORCEINLINE static const TShared<GenericWindow>& GetWindow()
		{
			return Get()->m_Window;
		}

		FORCEINLINE static const TShared<InputManager>& GetInputManager()
		{
			return Get()->m_InputManager;
		}

		FORCEINLINE static LayerStack* GetLayerStack()
		{
			return Get()->m_LayerStack.get();
		}

	protected:
		Application(App* clientApp);

		void Init();
		void Run();
		void Shutdown();

		// Event system related functions

		template<typename T>
		friend void PostEvent(const T& event);

		template<typename T>
		void PostEvent(const T& event)
		{
			DispatchEvent(event);
		}

		template<typename T>
		friend void PostDeferredEvent(const T& event);

		template<typename T>
		void PostDeferredEvent(const T& event)
		{
			m_EventQueue->PushEvent(event);
		}

		template<typename T>
		void DispatchEvent(const T& event)
		{
			TRACE_FUNCTION();

			m_EventDispatcher.Dispatch(event);

			m_InputManager->DispatchEvent(event);
			m_LayerStack->OnEvent(event);

			TRACE_BEGIN(0, "Application - Client::OnEvent");
			OnEvent(event);
			CallClientAppOnEvent(event);
			TRACE_END(0);
		}

		// Static event handler for EventQueue
		static inline void EventHandler(const Event& event)
		{
			TRACE_FUNCTION();

			Application::Get()->DispatchEvent(event);
		}

		// Event functions

		virtual void OnWindowCloseEvent_Internal(const WindowCloseEvent& event); // Virtual because it's overriden in WindowsApplication
		void OnWindowResizeEvent_Internal(const WindowResizeEvent& event);
		void OnWindowChangeDisplayModeEvent_Internal(const WindowChangeDisplayModeEvent& event);

		void OnKeyPressedEvent_Internal(const KeyPressedEvent& event);
		void OnKeyReleasedEvent_Internal(const KeyReleasedEvent& event);
		void OnKeyRepeatedEvent_Internal(const KeyRepeatedEvent& event);

		// @TODO: If possible, make this system set the event functions automatically based on function signatures
		// because writing this big type thing is a bit unnecessary 

		using ApplicationEventFunctions = TEventFunctionPack<
			TMemberEventFunction<Application, WindowCloseEvent, &Application::OnWindowCloseEvent_Internal>,
			TMemberEventFunction<Application, WindowResizeEvent, &Application::OnWindowResizeEvent_Internal>,
			TMemberEventFunction<Application, WindowChangeDisplayModeEvent, &Application::OnWindowChangeDisplayModeEvent_Internal>,
			TMemberEventFunction<Application, KeyPressedEvent, &Application::OnKeyPressedEvent_Internal>,
			TMemberEventFunction<Application, KeyReleasedEvent, &Application::OnKeyReleasedEvent_Internal>,
			TMemberEventFunction<Application, KeyRepeatedEvent, &Application::OnKeyRepeatedEvent_Internal>
		>;

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

		FORCEINLINE TShared<GenericWindow> GetApplicationWindow() const { return m_Window; }

		static Application* s_Instance;

	private:
		void CallClientAppOnEvent(const Event& event);
		float CalculateFrameTime(); // Implemented per platform

		void RenderToMainWindow();

		void SetupWindowTitle();
		void UpdateWindowTitle(float deltaTime);

		// @TODO: Move ImGui stuff to some generic Platform class

		void InitImGui() const;
		void ShutdownImGui() const;

		void SetImGuiCursor(int32 cursor);

		void LoadFonts();

	protected:
		virtual void InitImGuiBackend(const TShared<GenericWindow>& window) const { }
		virtual void ImGuiNewFramePlatform() const { }
		virtual void ImGuiRenderPlatform(ImDrawData* drawData) const { }
		virtual void ImGuiShutdownPlatform() const { }

	private:
		App* m_ClientApp;

		TShared<GenericWindow> m_Window;
		TShared<InputManager> m_InputManager;

		EventDispatcher<ApplicationEventFunctions, Application> m_EventDispatcher;
		TUnique<EventQueue<EventHandler>> m_EventQueue;
		TUnique<LayerStack> m_LayerStack;

		std::thread::id m_MainThreadId;

		//WString m_BaseWindowTitle;
		WString m_ApplicationTitle;

		EngineFonts m_Fonts;

		bool m_bRunning;

		template<typename T>
		friend void ParseCommandLineArgs(int32 argc, T* argv[]);
	};

	template<typename T>
	void ParseCommandLineArgs(int32 argc, T* argv[])
	{
		// @TODO: Save engine path in system environment variables or something

		for (int32 i = 0; i < argc; ++i)
		{
			bool bHasNextArg = i + 1 < argc;
			T* arg = argv[i];
			T* nextArg = bHasNextArg ? argv[i + 1] : nullptr;
			if (!tstrcmp(arg, STR_LITERAL_AS("--enginePath", T)) && bHasNextArg)
			{
				if constexpr (TIsSameV<T, char>)
				{
					EnginePath::SetEnginePath(StringConverter::StringToWString(nextArg));
				}
				else
				{
					EnginePath::SetEnginePath(nextArg);
				}
				i++;
			}
		}
	}

	template<typename T>
	inline Application* Application::Create(App* clientApp)
	{
		static_assert(TIsBaseOfV<Application, T>);
		ionassert(!s_Instance);
		return s_Instance = new T(clientApp);
	}

	inline EngineFonts& Application::GetEngineFonts()
	{
		return m_Fonts;
	}
}

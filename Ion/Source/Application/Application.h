#pragma once

#include "Core/Core.h"
#include "Core/Event/InputEvent.h"
#include "Core/Event/WindowEvent.h"
#include "Core/Layer/LayerStack.h"

#include "Application/Window/GenericWindow.h"

struct ImDrawData;

/* Specifies the main class of the application (can be used only once) */
#define USE_APPLICATION_CLASS(className) \
Ion::Application* Ion::CreateApplication() \
{ \
	return Application::Create<className>(); \
}

namespace Ion
{
	class EventQueue;
	class InputManager;

	class Renderer;

	class Shader;
	class IndexBuffer;

	class ION_API Application
	{
	public:
		using EventPtr = TShared<Event>;

		static WString GetEnginePath() { return s_EnginePath; }

		virtual ~Application() { }

		/* Called by the Entry Point */
		virtual void Start();

		/* Creates an instance of an application singleton */
		template<typename T>
		static TEnableIfT<TIsBaseOfV<Application, T>, T*> Create()
		{
			s_Instance = new T;
			return (T*)s_Instance;
		}

		static Application* Get();

		FORCEINLINE static const TShared<Renderer>& GetRenderer()
		{
			return Get()->m_Renderer;
		}

		FORCEINLINE static const TShared<GenericWindow>& GetWindow()
		{
			return Get()->m_Window;
		}

		FORCEINLINE static const TShared<InputManager>& GetInputManager()
		{
			return Get()->m_InputManager;
		}

	protected:
		Application();

		void Init();
		void Run();
		void Shutdown();

		void PostEvent(Event& event);
		void PostDeferredEvent(DeferredEvent& event);

		/* Platform specific method for polling application events / messages. */
		virtual void PollEvents();

		virtual void Update(float DeltaTime);
		virtual void Render();

		virtual void DispatchEvent(Event& event);

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
		virtual void OnEvent(Event& event) { }

		// End of overridables

		FORCEINLINE TShared<GenericWindow> GetApplicationWindow() const { return m_Window; }

		static Application* s_Instance;

	private:
		virtual float CalculateFrameTime();

		// @TODO: Move ImGui stuff to some generic Platform class

		void InitImGui() const;
		void ShutdownImGui() const;
		virtual void InitImGuiBackend(const TShared<GenericWindow>& window) const { }
		virtual void ImGuiNewFramePlatform() const { }
		virtual void ImGuiRenderPlatform(ImDrawData* drawData) const { }
		virtual void ImGuiShutdownPlatform() const { }

		bool m_bRunning;

		TShared<GenericWindow> m_Window;
		TShared<InputManager> m_InputManager;

		TShared<Renderer> m_Renderer;

		TUnique<EventQueue> m_EventQueue;
		TUnique<LayerStack> m_LayerStack;

		std::thread::id m_MainThreadId;

		WString m_BaseWindowTitle;

		template<typename T>
		friend void ParseCommandLineArgs(int32 argc, T* argv[]);
		static wchar* s_EnginePath;
	};

	Application* CreateApplication();

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
					int32 enginePathLength = (int32)strlen(nextArg);
					Application::s_EnginePath = new wchar[enginePathLength + 1];
					StringConverter::CharToWChar(nextArg, Application::s_EnginePath, enginePathLength + 1);
				}
				else
				{
					Application::s_EnginePath = nextArg;
				}
				i++;
			}
		}
	}
}

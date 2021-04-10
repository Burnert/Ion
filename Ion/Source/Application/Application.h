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

		virtual ~Application();

		void Init();

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

		/* Platform specific method for polling application events / messages. */
		virtual void PollEvents();

		virtual void Update(float DeltaTime);
		virtual void Render();

		void PostEvent(Event& event);
		void PostDeferredEvent(DeferredEvent& event);

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
		/* Main engine loop function */
		void Run();

		virtual float CalculateFrameTime();

		// @TODO: Move ImGui stuff to some generic Platform class

		void InitImGui() const;
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
	};

	Application* CreateApplication();
}

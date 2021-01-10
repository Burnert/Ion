#pragma once

#include "Core/Core.h"
#include "Core/Event/InputEvent.h"
#include "Core/Event/WindowEvent.h"
#include "Core/Layer/LayerStack.h"
#include "Application/Window/GenericWindow.h"

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

	class ION_API Application
	{
	public:
		using EventPtr = std::shared_ptr<Event>;

		virtual ~Application();

		void Init();

		/* Creates an instance of an application singleton */
		template<typename T>
		static std::enable_if_t<std::is_base_of_v<Application, T>, T*> Create()
		{
			s_Instance = new T;
			return (T*)s_Instance;
		}

		static Application* Get();

	protected:
		Application();

		/* Platform specific method for polling application events / messages. */
		virtual void PollEvents();

		virtual void Update(float DeltaTime);
		virtual void Render();

		virtual void OnEvent(Event& event);
		virtual void DispatchEvent(Event& event);

		// To be overriden in client:

		/* Override this in the client if you want to use it.
		   Runs after the engine has been initialised. (before the PostInit stage) */
		virtual void OnInit() { }
		/* Override this in the client if you want to use it.
		   Runs every frame. */
		virtual void OnUpdate(float DeltaTime) { }
		/* Override this in the client if you want to use it.
		   Runs every frame after the Update function. */
		virtual void OnRender() { }
		/* Override this in the client if you want to use it.
		   Runs after the engine has been shutdown and all the resources have been freed. */
		virtual void OnShutdown() { }
		/* Override this in the client if you want to use it.
		   Called when the application receives an event. */
		virtual void OnClientEvent(Event& event) { }

		// End of overridables

		FORCEINLINE std::shared_ptr<GenericWindow> GetApplicationWindow() const { return m_Window; }

		static Application* s_Instance;

	private:
		/* Main engine loop function */
		void RunMainLoop();

		bool m_bRunning = false;

		std::shared_ptr<GenericWindow> m_Window;
		std::shared_ptr<InputManager> m_InputManager;

		std::unique_ptr<EventQueue> m_EventQueue;
		std::unique_ptr<LayerStack> m_LayerStack;

		std::thread::id m_MainThreadId;
	};

	Application* CreateApplication();
}

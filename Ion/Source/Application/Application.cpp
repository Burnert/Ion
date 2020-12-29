#include "IonPCH.h"

#include "Application.h"

#include "Core/Event/InputEvent.h"
#include "Core/Event/EventQueue.h"
#include "Core/Event/EventDispatcher.h"

#define BIND_MEMBER_FUNC(x) std::bind(&x, this, std::placeholders::_1)

namespace Ion {

	Application::Application() :
		m_EventQueue(std::make_unique<EventQueue>())
	{
		m_EventQueue->SetEventHandler(BIND_MEMBER_FUNC(Application::DispatchEvent));
		Logger::Init();
	}

	Application::~Application() {}

	void Application::Init()
	{
		// Layer example

		using LayerPtr = std::shared_ptr<Layer>;
		m_LayerStack = std::make_unique<LayerStack>();
		LayerPtr layer1 = m_LayerStack->PushLayer<Layer>("Test1");
		LayerPtr overlay1 = m_LayerStack->PushOverlayLayer<Layer>("Overlay1");
		LayerPtr layer2 = m_LayerStack->PushLayer<Layer>("Test2");
		LayerPtr layer3 = m_LayerStack->PushLayer<Layer>("Test3");
		LayerPtr overlay2 = m_LayerStack->PushOverlayLayer<Layer>("Overlay2");

		auto findTest = m_LayerStack->GetLayer("Test2");
		ASSERT(findTest != nullptr)
		auto findTest2 = m_LayerStack->GetLayer("TestNotFound");
		ASSERT(findTest2 == nullptr)

		m_LayerStack->RemoveLayer("Test1");
		m_LayerStack->RemoveLayer("Overlay1");

		layer3->SetEnabled(false);

		// End Layer example

		// Create a platform specific window.
		m_ApplicationWindow = Ion::GenericWindow::Create();

		m_ApplicationWindow->SetEventCallback(BIND_MEMBER_FUNC(Application::OnEvent));

		m_ApplicationWindow->Initialize();
		m_ApplicationWindow->SetTitle(L"Ion Engine");

		m_ApplicationWindow->Show();

		m_bRunning = true;
		while (m_bRunning)
		{
			// Application loop
			PollEvents();

			Update(0.0f);
			Render();

			m_EventQueue->ProcessEvents();
		}
	}

	void Application::PollEvents()
	{
		// Platform specific
	}

	void Application::Update(float DeltaTime)
	{
		m_LayerStack->OnUpdate(DeltaTime);
	}

	void Application::Render()
	{
		m_LayerStack->OnRender();
	}

	void Application::OnEvent(Event& event)
	{
		if (event.IsDeferred())
			m_EventQueue->PushEvent(event.MakeShared());
		else
			DispatchEvent(event);
	}

	void Application::DispatchEvent(Event& event)
	{
		ION_LOG_ENGINE_DEBUG("Event: {0}", event.Debug_ToString());

		EventDispatcher dispatcher(event);

		// Window events

		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& event)
		{
			m_bRunning = false;
			return true;
		});

		m_LayerStack->OnEvent(event);
	}
}

#include "IonPCH.h"

#include "Application.h"

#include "Core/Event/InputEvent.h"
#include "Core/Event/EventQueue.h"
#include "Core/Event/EventDispatcher.h"
#include "Core/Input/Input.h"

#include "glad/glad.h"

#include "Core/Platform/PlatformCore.h"
#include "Application/Platform/Windows/WindowsApplication.h"

#include "RenderAPI/OpenGL/Windows/OpenGLWindows.h"

namespace Ion
{
	Application* Application::Get()
	{
		// This goes off when Application was not yet created.
		// (shouldn't ever happen, unless you're doing something weird)
		ASSERT(s_Instance);
		return s_Instance;
	}

	Application::Application() :
		m_EventQueue(MakeUnique<EventQueue>()),
		m_LayerStack(MakeUnique<LayerStack>()),
		m_MainThreadId(std::this_thread::get_id()),
		m_bRunning(true)
	{
		m_EventQueue->SetEventHandler(BIND_METHOD_1P(Application::DispatchEvent));
		Logger::Init();
	}

	Application::~Application() {}

	void Application::Init()
	{
		m_InputManager = InputManager::Create();

		InitRendererAPI();
		COUNTER_TIME_DATA(timeRenderApiInit, "RenderAPI_InitTime");
		LOG_INFO("{0}: {1}s", timeRenderApiInit.Name, ((float)timeRenderApiInit.GetTimeMs()) * 0.001f);

		// Create a platform specific window.
		m_Window = GenericWindow::Create();

		m_Window->SetEventCallback(BIND_METHOD_1P(Application::HandleEvent));

		

		m_Window->Initialize();
		m_Window->SetTitle(L"Ion Engine");

		// Current thread will render graphics in this window.
		m_Window->MakeRenderingContextCurrent();

		m_Window->Show();

		// Call client overriden Init function
		OnInit();

		uint vao;
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		float vertices[3 * 3] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.0f,  0.5f, 0.0f
		};
		uint vbo;
		glCreateBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
		glEnableVertexAttribArray(0);

		uint vertShader = glCreateShader(GL_VERTEX_SHADER);
		uint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

		const char* vertSrc = R"(
#version 430 core

layout(location = 0) in vec3 position;

void main()
{
	gl_Position = vec4(position, 1.0f);
}

)";
		glShaderSource(vertShader, 1, &vertSrc, 0);

		const char* fragSrc = R"(
#version 430 core

out vec4 color;

void main()
{
	color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}

)";
		glShaderSource(fragShader, 1, &fragSrc, 0);

		glCompileShader(vertShader);
		glCompileShader(fragShader);

		int bSuccess = 0;
		glGetShaderiv(vertShader, GL_COMPILE_STATUS, &bSuccess);
		ASSERT(bSuccess);
		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &bSuccess);
		ASSERT(bSuccess);

		uint program = glCreateProgram();
		glAttachShader(program, vertShader);
		glAttachShader(program, fragShader);

		glLinkProgram(program);
		int bLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &bLinked);
		ASSERT(bLinked);

		glDetachShader(program, vertShader);
		glDetachShader(program, fragShader);

		glUseProgram(program);

		RunMainLoop();

		// Call client overriden Shutdown function
		OnShutdown();
	}

	void Application::PollEvents()
	{
		// Platform specific
	}

	void Application::Update(float DeltaTime)
	{
		m_LayerStack->OnUpdate(DeltaTime);
		OnUpdate(DeltaTime);
	}

	void Application::Render()
	{
		m_LayerStack->OnRender();

		glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		OnRender();

		m_Window->SwapBuffers();
	}

	void Application::HandleEvent(Event& event)
	{
		if (event.IsDeferred())
			m_EventQueue->PushEvent(event.AsShared());
		else
			DispatchEvent(event);
	}

	void Application::DispatchEvent(Event& event)
	{
		ION_LOG_ENGINE_DEBUG("Event: {0}", event.Debug_ToString());

		EventDispatcher dispatcher(event);

		// Handle close event in application
		dispatcher.Dispatch<WindowCloseEvent>(
			[this](WindowCloseEvent& event)
			{
				m_bRunning = false;
				return false;
			});

		m_InputManager->OnEvent(event);
		m_LayerStack->OnEvent(event);
		OnEvent(event);
	}

	void Application::RunMainLoop()
	{
		// Application loop
		while (m_bRunning)
		{
			m_EventQueue->ProcessEvents();
			PollEvents();

			Update(0.0f);
			Render();
		}
	}

	Application* Application::s_Instance = nullptr;
}

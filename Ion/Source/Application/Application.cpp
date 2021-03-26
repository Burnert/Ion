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
		m_MainThreadId(std::this_thread::get_id())
	{
		m_EventQueue->SetEventHandler(BIND_METHOD_1P(Application::DispatchEvent));
		Logger::Init();
	}

	Application::~Application() {}

	void Application::Init()
	{
		m_InputManager = InputManager::Create();

		InitRendererAPI();
		COUNTER_TIME_DATA(timeRendererApiInit, "RendererAPI_InitTime");
		LOG_INFO("{0}: {1}s", timeRendererApiInit.Name, ((float)timeRendererApiInit.GetTimeMs()) * 0.001f);

		// Create a platform specific window.
		m_Window = GenericWindow::Create();

		m_Window->SetEventCallback(BIND_METHOD_1P(Application::OnEvent));

		

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

		int success = 0;
		glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
		ASSERT(success);
		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
		ASSERT(success);

		uint program = glCreateProgram();
		glAttachShader(program, vertShader);
		glAttachShader(program, fragShader);

		glLinkProgram(program);
		int isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		ASSERT(isLinked);

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

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		m_Window->SwapBuffers();

		OnRender();
	}

	void Application::OnEvent(Event& event)
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
		OnClientEvent(event);
	}

	void Application::RunMainLoop()
	{
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

	Application* Application::s_Instance = nullptr;
}

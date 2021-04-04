#include "IonPCH.h"

#include "Application.h"

#include "Core/Event/InputEvent.h"
#include "Core/Event/EventQueue.h"
#include "Core/Event/EventDispatcher.h"
#include "Core/Input/Input.h"

#include "glad/glad.h"

#include "RenderAPI/RenderAPI.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/Shader.h"

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

		InitRenderAPI();
		COUNTER_TIME_DATA(timeRenderApiInit, "RenderAPI_InitTime");
		LOG_INFO("{0}: {1}s", timeRenderApiInit.Name, ((float)timeRenderApiInit.GetTimeMs()) * 0.001f);

		// Create a platform specific window.
		m_Window = GenericWindow::Create();

		m_Window->Initialize();

		m_Window->SetEventCallback(BIND_METHOD_1P(Application::HandleEvent));

		// Current thread will render graphics in this window.
		m_Window->MakeRenderingContextCurrent();

		const char* renderAPILabel = RenderAPI::GetCurrentDisplayName();

		std::wstring windowTitle = TEXT("Ion - ");
		wchar renderAPILabelW[120];
		MultiByteToWideChar(CP_UTF8, 0, renderAPILabel, -1, renderAPILabelW, 120);
		windowTitle += renderAPILabelW;
		m_Window->SetTitle((wchar*)windowTitle.c_str());

		m_Window->Show();

		// Call client overriden Init function
		OnInit();

		uint vao;
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		};
		TShared<VertexBuffer> vbo = VertexBuffer::Create(vertices, sizeof(vertices));

		VertexLayout layout(2);
		layout.AddAttribute(EVertexAttributeType::Float, 3); // Position
		layout.AddAttribute(EVertexAttributeType::Float, 4); // Vertex Color

		vbo->SetLayout(layout);

		const char* vertSrc = R"(
#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

out vec4 v_Color;

void main()
{
	v_Color = color;
	gl_Position = vec4(position, 1.0f);
}

)";
		TShared<Shader> vertShader = Shader::Create(EShaderType::Vertex, vertSrc);

		const char* fragSrc = R"(
#version 430 core

in vec4 v_Color;

out vec4 color;

void main()
{
	color = v_Color;
}

)";
		TShared<Shader> fragShader = Shader::Create(EShaderType::Fragment, fragSrc);

		bool bResult;

		bResult = vertShader->Compile();
		ASSERT(bResult);
		bResult = fragShader->Compile();
		ASSERT(bResult);

		TShared<Program> program = Program::Create();
		
		program->AttachShader(vertShader);
		program->AttachShader(fragShader);

		bResult = program->Link();
		ASSERT(bResult);

		program->Bind();

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
		//ION_LOG_ENGINE_DEBUG("Event: {0}", event.Debug_ToString());

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

	void Application::InitRenderAPI()
	{
		RenderAPI::Init(ERenderAPI::OpenGL);
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

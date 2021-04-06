#include "IonPCH.h"

#include "Application.h"

#include "Core/Event/InputEvent.h"
#include "Core/Event/EventQueue.h"
#include "Core/Event/EventDispatcher.h"
#include "Core/Input/Input.h"

#include "glad/glad.h"

#include "RenderAPI/RenderAPI.h"
#include "Renderer/Renderer.h"

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

		// Current thread will render graphics in this window.
		m_Window->Initialize();

		m_Window->SetEventCallback(BIND_METHOD_1P(Application::HandleEvent));

		m_Renderer = Renderer::Create();
		m_Renderer->Init();

		m_Renderer->SetVSyncEnabled(true);

		const char* renderAPILabel = RenderAPI::GetCurrentDisplayName();

		std::wstring windowTitle = TEXT("Ion - ");
		wchar renderAPILabelW[120];
		// @TODO: This is from Windows, change that:
		MultiByteToWideChar(CP_UTF8, 0, renderAPILabel, -1, renderAPILabelW, 120);
		windowTitle += renderAPILabelW;
		m_Window->SetTitle((wchar*)windowTitle.c_str());

		m_Window->Show();

		// Call client overriden Init function
		OnInit();

		float vertices[4 * 7] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		};
		TShared<VertexBuffer> vbo = VertexBuffer::Create(vertices, sizeof(vertices));

		VertexLayout layout(2);
		layout.AddAttribute(EVertexAttributeType::Float, 3); // Position
		layout.AddAttribute(EVertexAttributeType::Float, 4); // Vertex Color

		vbo->SetLayout(layout);

		uint indices[2 * 3] = {
			0, 1, 2,
			2, 3, 0,
		};
		TShared<IndexBuffer> ibo = IndexBuffer::Create(indices, 2 * 3);

		float cubeVerts[8 * 7] = {
			-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
			-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
		};
		TShared<VertexBuffer> vboCube = VertexBuffer::Create(cubeVerts, sizeof(cubeVerts));
		vboCube->SetLayout(layout);

		uint cubeIndices[12 * 3] = {
			// Front
			0, 1, 2,
			2, 3, 0,

			// Back
			4, 5, 6,
			6, 7, 4,

			// Right
			1, 4, 7,
			7, 2, 1,

			// Left
			5, 0, 3,
			3, 6, 5,

			// Top
			3, 2, 7,
			7, 6, 3,

			// Bottom
			5, 4, 1,
			5, 1, 0,
		};
		TShared<IndexBuffer> iboCube = IndexBuffer::Create(cubeIndices, 12 * 3);

		const char* vertSrc = R"(
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

uniform mat4 u_MVP;
uniform vec4 u_Color;

out vec4 v_Color;

void main()
{
	v_Color = a_Color * u_Color;
	gl_Position = u_MVP * vec4(a_Position, 1.0f);
}

)";

		const char* fragSrc = R"(
#version 430 core

in vec4 v_Color;

out vec4 color;

void main()
{
	color = v_Color;
}

)";

		bool bResult;

		TShared<Shader> shader = Shader::Create();
		shader->AddShaderSource(EShaderType::Vertex, vertSrc);
		shader->AddShaderSource(EShaderType::Pixel, fragSrc);

		bResult = shader->Compile();
		VERIFY(bResult);

		shader->Bind();

		m_Shader = shader;

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
		static float c_Angle = 0.0f;
		static FVector4 c_Tint(1.0f, 0.0f, 1.0f, 1.0f);

		FVector4 tint = c_Tint;
		tint.y = glm::abs(tint.y - 1.0f);
		m_Shader->SetUniform4f("u_Color", tint);

		// Perspective projection
		float fov = glm::radians(90.0f);
		WindowDimensions dimensions = m_Window->GetDimensions();
		float aspectRatio = (float)dimensions.Width / (float)dimensions.Height;
		FMatrix4 projectionMatrix = glm::perspective(fov, aspectRatio, 0.1f, 100.0f);

		// Inverted camera
		FMatrix4 viewMatrix = glm::translate(FVector3(0.0f, 0.0f, -2.0f));

		// Model transform
		FMatrix4 modelMatrix(1.0f);
		//modelMatrix *= glm::translate(FVector3(0.3f, 0.5f, 0.0f));
		modelMatrix *= glm::rotate(c_Angle, glm::normalize(FVector3(1.0f, 0.0f, 1.0f)));

		// Multiply in this order because OpenGL
		FMatrix4 modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;

		m_Shader->SetUniformMatrix4f("u_MVP", modelViewProjectionMatrix);

		m_LayerStack->OnUpdate(DeltaTime);
		OnUpdate(DeltaTime);

		c_Angle += 0.01f;
		c_Tint.y = (((c_Tint.y + 0.01f) >= 2.0f) ? 0.0f : (c_Tint.y + 0.01f));
	}

	void Application::Render()
	{
		m_Renderer->Clear(FVector4(0.1f, 0.1f, 0.1f, 1.0f));

		glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, nullptr);

		m_LayerStack->OnRender();
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

		dispatcher.Dispatch<WindowResizeEvent>(
			[this](WindowResizeEvent& event)
			{
				uint width = event.GetWidth();
				uint height = event.GetHeight();

				// @TODO: Move this to the Renderer impl? RenderAPI impl? Wherever.
				glViewport(0, 0, width, height);

				return true;
			});

		dispatcher.Dispatch<KeyPressedEvent>(
			[this](KeyPressedEvent& event)
			{
				if (!event.IsRepeated())
				{
					// Toggle wireframe display with W key
					if (event.GetKeyCode() == Key::W)
					{
						uint mode;
						glGetIntegerv(GL_POLYGON_MODE, (int*)&mode);
						if (mode == GL_FILL)
							glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						else
							glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					}
					else if (event.GetKeyCode() == Key::V)
					{
						static bool c_VSync = true;
						c_VSync = !c_VSync;
						m_Renderer->SetVSyncEnabled(c_VSync);
					}
				}
				return true;
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

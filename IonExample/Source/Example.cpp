#include "IonPCH.h"

#include "Ion.h"
#include "Renderer/Renderer.h"

//DECLARE_PERFORMANCE_COUNTER(Counter_MainLoop, "Main Loop", "Client");
//DECLARE_PERFORMANCE_COUNTER(Counter_MainLoop_Section, "Main Loop Section", "Client");

//DECLARE_PERFORMANCE_COUNTER_GENERIC(Counter_FileI, "FileI");
//DECLARE_PERFORMANCE_COUNTER_GENERIC(Counter_FileIC, "FileIC");
//DECLARE_PERFORMANCE_COUNTER_GENERIC(Counter_FileCpp, "Fcpp");

// I'll think about this...
using namespace Ion;

class IonExample : public IonApplication
{
public:
	IonExample()
	{
		ION_LOG_DEBUG("IonExample constructed.");
	}

	virtual void OnInit() override
	{
		//Ion::SerialisationTest();

		Ion::File* file = Ion::File::Create(TEXT("linetest.txt"));
		file->Open(Ion::IO::FM_Read);

		std::string line;
		while (!file->EndOfFile())
		{
			file->ReadLine(line);
			LOG_INFO("Read Line: {0}", line);
		}

// #include "FileTest"

		VertexLayout layout(2);
		layout.AddAttribute(EVertexAttributeType::Float, 3); // Position
		layout.AddAttribute(EVertexAttributeType::Float, 4); // Vertex Color

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
		m_CubeVertexBuffer = VertexBuffer::Create(cubeVerts, sizeof(cubeVerts));
		m_CubeVertexBuffer->SetLayout(layout);

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
		m_CubeIndexBuffer = IndexBuffer::Create(cubeIndices, 12 * 3);

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
	}

	virtual void OnUpdate(float deltaTime) override
	{
		//{
		//	SCOPED_PERFORMANCE_COUNTER(Counter_MainLoop);

		//	using namespace std::chrono_literals;
		//	std::this_thread::sleep_for(0.2s);

		//	MANUAL_PERFORMANCE_COUNTER(Counter_MainLoop_Section);
		//	Counter_MainLoop_Section->Start();

		//	std::this_thread::sleep_for(0.2s);

		//	Counter_MainLoop_Section->Stop();
		//}

		//COUNTER_TIME_DATA(dataMainLoop, "Counter_MainLoop");
		//COUNTER_TIME_DATA(dataMainLoopSection, "Counter_MainLoop_Section");

		//LOG_WARN("{0} Data: {1}", dataMainLoop.Name, dataMainLoop.GetTimeMs());
		//LOG_WARN("{0} Data: {1}", dataMainLoopSection.Name, dataMainLoopSection.GetTimeMs());

		static float c_Angle = 0.0f;
		static FVector4 c_Tint(1.0f, 0.0f, 1.0f, 1.0f);

		FVector4 tint = c_Tint;
		tint.y = glm::abs(tint.y - 1.0f);
		m_Shader->SetUniform4f("u_Color", tint);

		// Perspective projection
		float fov = glm::radians(90.0f);
		WindowDimensions dimensions = GetWindow()->GetDimensions();
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

		c_Angle += deltaTime;
		c_Tint.y = (((c_Tint.y + deltaTime) >= 2.0f) ? 0.0f : (c_Tint.y + deltaTime));
	}

	virtual void OnRender() override
	{
		GetRenderer()->Clear(FVector4(0.1f, 0.1f, 0.1f, 1.0f));
		// @TODO: Now an IndexBuffer is completely independent of a VertexBuffer,
		// so when this gets called it won't bind the VBO before drawing.
		// Hence this is a bug.
		// > Think of a better system.
		GetRenderer()->Draw(m_CubeIndexBuffer);
	}

	virtual void OnShutdown() override
	{
	}

	virtual void OnEvent(Ion::Event& event) override
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<KeyPressedEvent>(
			[this](KeyPressedEvent& event)
			{
				// Toggle wireframe display with W key
				if (event.GetKeyCode() == Key::W)
				{
					EPolygonDrawMode drawMode = (GetRenderer()->GetPolygonDrawMode() == EPolygonDrawMode::Fill) ?
						EPolygonDrawMode::Lines : EPolygonDrawMode::Fill;
					GetRenderer()->SetPolygonDrawMode(drawMode);
				}
				// Toggle VSync with V key
				else if (event.GetKeyCode() == Key::V)
				{
					bool vsync = GetRenderer()->GetVSyncEnabled();
					GetRenderer()->SetVSyncEnabled(!vsync);
				}
				return true;
			});
	}

private:
	TShared<VertexBuffer> m_CubeVertexBuffer;
	TShared<IndexBuffer> m_CubeIndexBuffer;
	TShared<Shader> m_Shader;
};

USE_APPLICATION_CLASS(IonExample)

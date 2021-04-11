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

		//Ion::File* file = Ion::File::Create(TEXT("linetest.txt"));
		//file->Open(Ion::IO::FM_Read);

		//std::string line;
		//while (!file->EndOfFile())
		//{
		//	file->ReadLine(line);
		//	LOG_INFO("Read Line: {0}", line);
		//}

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
		TShared<VertexBuffer> vertexBuffer = VertexBuffer::Create(cubeVerts, sizeof(cubeVerts));
		vertexBuffer->SetLayout(layout);

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
		TShared<IndexBuffer> indexBuffer = IndexBuffer::Create(cubeIndices, 12 * 3);

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

		m_MeshCube = Mesh::Create();
		m_MeshCube->SetVertexBuffer(vertexBuffer);
		m_MeshCube->SetIndexBuffer(indexBuffer);
		m_MeshCube->SetShader(shader);

		m_Camera = Camera::Create();
		m_Camera->SetTransform(glm::translate(FVector3(0.0f, 0.0f, 2.0f)));

		float fov = glm::radians(90.0f);
		m_Camera->SetFOV(fov);
		m_Camera->SetNearClip(0.1f);
		m_Camera->SetFarClip(100.0f);
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

		TShared<Shader> cubeShader = m_MeshCube->GetShader();

		static float c_Angle = 0.0f;
		static FVector4 c_Tint(1.0f, 0.0f, 1.0f, 1.0f);

		FVector4 tint = c_Tint;
		tint.y = glm::abs(tint.y - 1.0f);
		cubeShader->SetUniform4f("u_Color", tint);

		// Perspective projection
		WindowDimensions dimensions = GetWindow()->GetDimensions();
		float aspectRatio = (float)dimensions.Width / (float)dimensions.Height;
		m_Camera->SetAspectRatio(aspectRatio);

		float cameraMoveSpeed = 5.0f;

		FVector4 cameraForwardVector = 
			glm::rotate(m_CameraRotation.y, FVector3(0.0f, -1.0f, 0.0f)) 
			* glm::rotate(m_CameraRotation.x, FVector3(-1.0f, 0.0f, 0.0f))
			* FVector4(0.0f, 0.0f, -1.0f, 0.0f);

		FVector4 cameraRightVector =
			glm::rotate(m_CameraRotation.y, FVector3(0.0f, -1.0f, 0.0f))
			* glm::rotate(m_CameraRotation.x, FVector3(-1.0f, 0.0f, 0.0f))
			* FVector4(-1.0f, 0.0f, 0.0f, 0.0f);

		FVector4 cameraUpVector =
			glm::rotate(m_CameraRotation.y, FVector3(0.0f, -1.0f, 0.0f))
			* glm::rotate(m_CameraRotation.x, FVector3(-1.0f, 0.0f, 0.0f))
			* FVector4(0.0f, -1.0f, 0.0f, 0.0f);

		if (GetInputManager()->IsKeyPressed(Key::W))
		{
			m_CameraLocation = glm::translate(FVector3(cameraForwardVector) * deltaTime * cameraMoveSpeed) * m_CameraLocation;
		}
		if (GetInputManager()->IsKeyPressed(Key::S))
		{
			m_CameraLocation = glm::translate(FVector3(cameraForwardVector) * -deltaTime * cameraMoveSpeed) * m_CameraLocation;
		}
		if (GetInputManager()->IsKeyPressed(Key::A))
		{
			m_CameraLocation = glm::translate(FVector3(cameraRightVector) * deltaTime * cameraMoveSpeed) * m_CameraLocation;
		}
		if (GetInputManager()->IsKeyPressed(Key::D))
		{
			m_CameraLocation = glm::translate(FVector3(cameraRightVector) * -deltaTime * cameraMoveSpeed) * m_CameraLocation;
		}
		if (GetInputManager()->IsKeyPressed(Key::Q))
		{
			m_CameraLocation = glm::translate(FVector3(cameraUpVector) * deltaTime * cameraMoveSpeed) * m_CameraLocation;
		}
		if (GetInputManager()->IsKeyPressed(Key::E))
		{
			m_CameraLocation = glm::translate(FVector3(cameraUpVector) * -deltaTime * cameraMoveSpeed) * m_CameraLocation;
		}

		FMatrix4 transform(1.0f);
		transform *= glm::translate(FVector3(m_CameraLocation));
		transform *= glm::rotate(m_CameraRotation.y, FVector3(0.0f, -1.0f, 0.0f));
		transform *= glm::rotate(m_CameraRotation.x, FVector3(-1.0f, 0.0f, 0.0f));
		m_Camera->SetTransform(transform);

		const FMatrix4& viewProjectionMatrix = m_Camera->UpdateViewProjectionMatrix();

		// Model transform
		FMatrix4 modelMatrix(1.0f);
		modelMatrix *= glm::rotate(c_Angle, glm::normalize(FVector3(1.0f, 0.0f, 1.0f)));
		m_MeshCube->SetTransform(modelMatrix);

		// Multiply in this order because OpenGL
		FMatrix4 modelViewProjectionMatrix = viewProjectionMatrix * m_MeshCube->GetTransform();

		cubeShader->SetUniformMatrix4f("u_MVP", modelViewProjectionMatrix);

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
		GetRenderer()->Draw(m_MeshCube);
	}

	virtual void OnShutdown() override
	{
	}

	virtual void OnEvent(Ion::Event& event) override
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<RawInputMouseMovedEvent>(
			[this](RawInputMouseMovedEvent& event)
			{
				float yawDelta = event.GetX() * 0.003f;
				float pitchDelta = event.GetY() * 0.003f;

				m_CameraRotation.x += pitchDelta;
				m_CameraRotation.y += yawDelta;

				return true;
			});
	}

private:
	TShared<Mesh> m_MeshCube;
	TShared<Camera> m_Camera;
	FVector4 m_CameraLocation = { 0.0f, 0.0f, 2.0f, 1.0f };
	FVector3 m_CameraRotation = { 0.0f, 0.0f, 0.0f };
};

USE_APPLICATION_CLASS(IonExample)

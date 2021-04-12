#include "IonPCH.h"

#include "Ion.h"
#include "Renderer/Renderer.h"
#include "UserInterface/ImGui.h"

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
		layout.AddAttribute(EVertexAttributeType::Float, 2); // Texture Coordinate
		layout.AddAttribute(EVertexAttributeType::Float, 4); // Vertex Color

		float cubeVerts[8 * 9] = {
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
			-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
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
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec4 a_Color;

uniform mat4 u_MVP;
uniform vec4 u_Color;

out vec4 v_Color;
out vec2 v_TexCoord;

void main()
{
	v_Color = a_Color * u_Color;
	v_TexCoord = a_TexCoord;
	gl_Position = u_MVP * vec4(a_Position, 1.0f);
}

)";

		const char* fragSrc = R"(
#version 430 core

uniform sampler2D u_TextureSampler;

in vec4 v_Color;
in vec2 v_TexCoord;

out vec4 Color;

void main()
{
	Color = texture(u_TextureSampler, v_TexCoord).rgba * v_Color;
}

)";

		bool bResult;

		TShared<Shader> shader = Shader::Create();
		shader->AddShaderSource(EShaderType::Vertex, vertSrc);
		shader->AddShaderSource(EShaderType::Pixel, fragSrc);

		bResult = shader->Compile();
		VERIFY(bResult);

		// @TODO: Make some kind of converter from String to WString and vice-versa because this is painful to write:

		memset(m_TextureFileNameBuffer, 0, sizeof(m_TextureFileNameBuffer));
		WString textureFileName = L"test.png";
		char* textureFileNameA = new char[textureFileName.size() + 1];
		WideCharToMultiByte(CP_UTF8, 0, textureFileName.c_str(), -1, textureFileNameA, (int)textureFileName.size() + 1, nullptr, nullptr);
		strcpy_s(m_TextureFileNameBuffer, textureFileNameA);
		delete[] textureFileNameA;

		File* textureFile = File::Create(textureFileName);
		Image* textureImage = new Image;
		VERIFY(textureImage->Load(textureFile));
		delete textureFile;
		m_Texture = Texture::Create(textureImage);
		m_Texture->Bind(0);

		shader->SetUniform1i("u_TextureSampler", 0);

		TShared<Material> material = Material::Create();
		material->SetShader(shader);
		material->CreateMaterialProperty("Color", EMaterialPropertyType::Float4);
		material->LinkPropertyToUniform("Color", "u_Color");
		material->SetMaterialProperty("Color", FVector4(1.0f, 1.0f, 1.0f, 1.0f));

		m_MeshCube = Mesh::Create();
		m_MeshCube->SetVertexBuffer(vertexBuffer);
		m_MeshCube->SetIndexBuffer(indexBuffer);
		m_MeshCube->SetMaterial(material);

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

		TShared<Shader> cubeShader = m_MeshCube->GetMaterial()->GetShader();
		TShared<Material> cubeMaterial = m_MeshCube->GetMaterial();

		static float c_Angle = 0.0f;
		static FVector4 c_Tint(1.0f, 0.0f, 1.0f, 1.0f);

		FVector4 tint = c_Tint;
		tint.y = glm::abs(tint.y - 1.0f);
		cubeMaterial->SetMaterialProperty("Color", tint);

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

		if (GetInputManager()->IsMouseButtonPressed(Mouse::Right))
		{
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

		ImGui::Begin("Texture settings");
		ImGui::InputText("Texture file", m_TextureFileNameBuffer, sizeof(m_TextureFileNameBuffer));
		if (ImGui::Button("Set Texture"))
		{
			int bufferSize = MultiByteToWideChar(CP_UTF8, 0, m_TextureFileNameBuffer, -1, 0, 0);
			wchar* textureFileNameBufferW = new wchar[bufferSize];
			MultiByteToWideChar(CP_UTF8, 0, m_TextureFileNameBuffer, -1, textureFileNameBufferW, bufferSize);

			File* textureFile = File::Create(textureFileNameBufferW);
			delete[] textureFileNameBufferW;
			if (textureFile->Exists())
			{
				Image* textureImage = new Image;
				VERIFY(textureImage->Load(textureFile));
				m_Texture = Texture::Create(textureImage);
				m_Texture->Bind(0);
			}
			delete textureFile;
		}
		ImGui::End();
	}

	virtual void OnRender() override
	{
		GetRenderer()->Clear(FVector4(0.1f, 0.1f, 0.1f, 1.0f));
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
				if (GetInputManager()->IsMouseButtonPressed(Mouse::Right))
				{
					float yawDelta = event.GetX() * 0.003f;
					float pitchDelta = event.GetY() * 0.003f;

					m_CameraRotation.x += pitchDelta;
					m_CameraRotation.y += yawDelta;

					return true;
				}
				return false;
			});

		dispatcher.Dispatch<MouseButtonPressedEvent>(
			[this](MouseButtonPressedEvent& event)
			{
				if (event.GetMouseButton() == Mouse::Right)
				{
					GetWindow()->LockCursor();
					GetWindow()->ShowCursor(false);
					return true;
				}
				return false;
			});

		dispatcher.Dispatch<MouseButtonReleasedEvent>(
			[this](MouseButtonReleasedEvent& event)
			{
				if (event.GetMouseButton() == Mouse::Right)
				{
					GetWindow()->UnlockCursor();
					GetWindow()->ShowCursor(true);
					return true;
				}
				return false;
			});
	}

private:
	TShared<Mesh> m_MeshCube;
	TShared<Camera> m_Camera;
	TShared<Texture> m_Texture;
	FVector4 m_CameraLocation = { 0.0f, 0.0f, 2.0f, 1.0f };
	FVector3 m_CameraRotation = { 0.0f, 0.0f, 0.0f };

	char m_TextureFileNameBuffer[MAX_PATH + 1];
};

USE_APPLICATION_CLASS(IonExample)

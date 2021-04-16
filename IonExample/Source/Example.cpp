#include "IonPCH.h"

#include "Ion.h"
#include "Renderer/Renderer.h"
#include "UserInterface/ImGui.h"

// I'll think about this...
using namespace Ion;

class IonExample : public IonApplication
{
public:
	IonExample()
		: m_TextureFileNameBuffer("")
	{
		ION_LOG_DEBUG("IonExample constructed.");
	}

	virtual void OnInit() override
	{
		VertexLayout layout(3);
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
		ionassert(bResult);

		WString textureFileName = L"test.png";
		memset(m_TextureFileNameBuffer, 0, sizeof(m_TextureFileNameBuffer));
		StringConverter::WCharToChar(textureFileName.c_str(), m_TextureFileNameBuffer);

		File* textureFile = File::Create(textureFileName);
		Image* textureImage = new Image;
		ionassertnd(textureImage->Load(textureFile));
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
		m_Camera->SetFOV(glm::radians(90.0f));
		m_Camera->SetNearClip(0.1f);
		m_Camera->SetFarClip(100.0f);

		m_Scene = Scene::Create();
		m_Scene->SetActiveCamera(m_Camera);
		m_Scene->AddDrawableObject(m_MeshCube);

		m_AuxCamera = Camera::Create();
		m_AuxCamera->SetTransform(glm::translate(FVector3(0.0f, 0.0f, 4.0f)));
		m_AuxCamera->SetFOV(glm::radians(66.0f));
		m_AuxCamera->SetNearClip(0.1f);
		m_AuxCamera->SetFarClip(10.0f);
	}

	virtual void OnUpdate(float deltaTime) override
	{
		TShared<Shader> cubeShader = m_MeshCube->GetMaterial()->GetShader();
		TShared<Material> cubeMaterial = m_MeshCube->GetMaterial();

		static float c_Angle = 0.0f;
		static FVector4 c_Tint(1.0f, 0.0f, 1.0f, 1.0f);

		FVector4 tint = c_Tint;
		tint.y = glm::abs(tint.y - 1.0f);
		cubeMaterial->SetMaterialProperty("Color", tint);

		// Perspective projection
		WindowDimensions dimensions = GetWindow()->GetDimensions();
		float aspectRatio = dimensions.GetAspectRatio();
		m_Camera->SetAspectRatio(aspectRatio);
		m_AuxCamera->SetAspectRatio(aspectRatio);

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

		// Model transform
		FMatrix4 modelMatrix(1.0f);
		modelMatrix *= glm::translate(m_MeshLocation);
		modelMatrix *= glm::rotate(c_Angle, glm::normalize(FVector3(0.0f, 1.0f, 0.0f)));
		m_MeshCube->SetTransform(modelMatrix);

		c_Angle += deltaTime;
		c_Tint.y = (((c_Tint.y + deltaTime) >= 2.0f) ? 0.0f : (c_Tint.y + deltaTime));


		m_AuxCamera->SetTransform(glm::translate(m_AuxCameraLocation));

		// ImGui:

		ImGui::Begin("Diagnostics");
		{
			if (ImGui::Button("Start profiling"))
			{
				TRACE_RECORD_START();
			}
			if (ImGui::Button("Stop profiling"))
			{
				TRACE_RECORD_STOP();
			}
		}
		ImGui::End();

		ImGui::Begin("Texture settings");
		{
			ImGui::InputText("Texture file", m_TextureFileNameBuffer, sizeof(m_TextureFileNameBuffer));
			if (ImGui::Button("Set Texture"))
			{
				File* textureFile = File::Create(StringConverter::StringToWString(m_TextureFileNameBuffer));
				if (textureFile->Exists())
				{
					Image* textureImage = new Image;
					ionassertnd(textureImage->Load(textureFile));
					m_Texture = Texture::Create(textureImage);
					m_Texture->Bind(0);
				}
				delete textureFile;
			}
			ImGui::DragFloat3("Aux Camera Location", &m_AuxCameraLocation.x, 0.01f, -FLT_MAX, FLT_MAX);
			if (ImGui::Button("Change Camera"))
			{
				m_Scene->SetActiveCamera(m_Scene->GetActiveCamera() == m_Camera ? m_AuxCamera : m_Camera);
			}
		}
		ImGui::End();

		ImGui::Begin("Mesh settings");
		{
			ImGui::DragFloat3("Location", &m_MeshLocation.x, 0.01f, -FLT_MAX, FLT_MAX);
		}
		ImGui::End();
	}

	virtual void OnRender() override
	{
		GetRenderer()->Clear(FVector4(0.1f, 0.1f, 0.1f, 1.0f));
		GetRenderer()->RenderScene(m_Scene);
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

		dispatcher.Dispatch<RawInputMouseButtonPressedEvent>(
			[this](RawInputMouseButtonPressedEvent& event)
			{
				if (event.GetMouseButton() == Mouse::Right)
				{
					ImGui::SetWindowFocus();

					GetWindow()->LockCursor();
					GetWindow()->ShowCursor(false);
					return true;
				}
				return false;
			});

		dispatcher.Dispatch<RawInputMouseButtonReleasedEvent>(
			[this](RawInputMouseButtonReleasedEvent& event)
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
	TShared<Camera> m_AuxCamera;
	TShared<Texture> m_Texture;
	TShared<Scene> m_Scene;
	FVector4 m_CameraLocation = { 0.0f, 0.0f, 2.0f, 1.0f };
	FVector3 m_CameraRotation = { 0.0f, 0.0f, 0.0f };
	FVector3 m_MeshLocation = { 0.0f, 0.0f, 0.0f };
	FVector3 m_AuxCameraLocation = { 0.0f, 0.0f, 4.0f };

	char m_TextureFileNameBuffer[MAX_PATH + 1];
};

USE_APPLICATION_CLASS(IonExample);

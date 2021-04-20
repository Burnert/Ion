#include "IonPCH.h"

#include "Ion.h"
#include "Renderer/Renderer.h"
#include "UserInterface/ImGui.h"

#include "Core/File/Collada.h"

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
		const char* vertSrc = R"(
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord0;

uniform mat4 u_MVP;
uniform mat4 u_Transform;
uniform mat4 u_InverseTranspose;
uniform vec3 u_CameraLocation;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_WorldNormal;

void main()
{
	gl_Position = u_MVP * vec4(a_Position, 1.0);
	v_TexCoord = a_TexCoord0;

	v_Normal = a_Normal;
	v_WorldNormal = normalize(vec3(u_InverseTranspose * vec4(v_Normal, 0.0)));
}

)";

		const char* fragSrc = R"(
#version 430 core

uniform sampler2D u_TextureSampler;
uniform vec3 u_LightDirection;

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_WorldNormal;

out vec4 Color;

void main()
{
	float surfaceLight = dot(v_WorldNormal, -u_LightDirection);

	Color = texture(u_TextureSampler, v_TexCoord).rgba * surfaceLight;

	//Color = vec4((v_WorldNormal + 1.0) * 0.5, 1.0);
}

)";

		bool bResult;

		TShared<Shader> shader = Shader::Create();
		shader->AddShaderSource(EShaderType::Vertex, vertSrc);
		shader->AddShaderSource(EShaderType::Pixel, fragSrc);

		bResult = shader->Compile();
		ionassert(bResult);

		m_Camera = Camera::Create();
		m_Camera->SetTransform(glm::translate(FVector3(0.0f, 0.0f, 2.0f)));
		m_Camera->SetFOV(glm::radians(90.0f));
		m_Camera->SetNearClip(0.1f);
		m_Camera->SetFarClip(100.0f);

		m_Scene = Scene::Create();
		m_Scene->SetActiveCamera(m_Camera);

		m_AuxCamera = Camera::Create();
		m_AuxCamera->SetTransform(glm::translate(FVector3(0.0f, 0.0f, 4.0f)));
		m_AuxCamera->SetFOV(glm::radians(66.0f));
		m_AuxCamera->SetNearClip(0.1f);
		m_AuxCamera->SetFarClip(10.0f);

		// @TODO: Create an asset manager for textures, meshes and other files that can be imported

		TUnique<File> colladaFile = TUnique<File>(File::Create(L"char.dae"));
		ColladaDocument colladaDoc(colladaFile.get());

		//TUnique<File> colladaStressTestFile = TUnique<File>(File::Create(L"spherestresstest.dae"));
		//ColladaDocument colladaStressTest(colladaStressTestFile.get());

		const ColladaData& colladaData = colladaDoc.GetData();

		TShared<VertexBuffer> colladaVertexBuffer = VertexBuffer::Create(colladaData.VertexAttributes, colladaData.VertexAttributeCount);
		colladaVertexBuffer->SetLayout(colladaData.Layout);

		TShared<IndexBuffer> colladaIndexBuffer = IndexBuffer::Create(colladaData.Indices, (uint)colladaData.IndexCount);

		WString textureFileName = L"char.png";
		memset(m_TextureFileNameBuffer, 0, sizeof(m_TextureFileNameBuffer));
		StringConverter::WCharToChar(textureFileName.c_str(), m_TextureFileNameBuffer);

		File* textureFile = File::Create(textureFileName);
		Image* textureImage = new Image;
		ionassertnd(textureImage->Load(textureFile));
		delete textureFile;
		m_TextureCollada = Texture::Create(textureImage);
		m_TextureCollada->Bind(1);

		TShared<Material> material = Material::Create();
		material->SetShader(shader);

		shader->SetUniform1i("u_TextureSampler", 1);

		m_MeshCollada = Mesh::Create();
		m_MeshCollada->SetVertexBuffer(colladaVertexBuffer);
		m_MeshCollada->SetIndexBuffer(colladaIndexBuffer);
		m_MeshCollada->SetMaterial(material);
		m_MeshCollada->SetTransform(glm::rotate(glm::radians(-90.0f), FVector3(1.0f, 0.0f, 0.0f)));

		m_Scene->AddDrawableObject(m_MeshCollada);
	}

	virtual void OnUpdate(float deltaTime) override
	{
		TShared<Material> meshMaterial = m_MeshCollada->GetMaterial();
		TShared<Shader> meshShader = meshMaterial->GetShader();

		static float c_Angle = 0.0f;
		static FVector4 c_Tint(1.0f, 0.0f, 1.0f, 1.0f);

		// Perspective projection
		WindowDimensions dimensions = GetWindow()->GetDimensions();
		float aspectRatio = dimensions.GetAspectRatio();
		m_Camera->SetAspectRatio(aspectRatio);
		m_AuxCamera->SetAspectRatio(aspectRatio);

		m_LightDirection = glm::normalize(m_LightDirection);
		meshShader->SetUniform3f("u_LightDirection", m_LightDirection);

		float cameraMoveSpeed = 5.0f;

		float cameraPitch = glm::radians(m_CameraRotation.x);
		float cameraYaw = glm::radians(m_CameraRotation.y);

		FVector4 cameraForwardVector = 
			glm::rotate(cameraYaw, FVector3(0.0f, -1.0f, 0.0f))
			* glm::rotate(cameraPitch, FVector3(-1.0f, 0.0f, 0.0f))
			* FVector4(0.0f, 0.0f, -1.0f, 0.0f);

		FVector4 cameraRightVector =
			glm::rotate(cameraYaw, FVector3(0.0f, -1.0f, 0.0f))
			* glm::rotate(cameraPitch, FVector3(-1.0f, 0.0f, 0.0f))
			* FVector4(-1.0f, 0.0f, 0.0f, 0.0f);

		FVector4 cameraUpVector =
			glm::rotate(cameraYaw, FVector3(0.0f, -1.0f, 0.0f))
			* glm::rotate(cameraPitch, FVector3(-1.0f, 0.0f, 0.0f))
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
				m_CameraLocation = glm::translate(FVector3(0.0f, 1.0f, 0.0f) * -deltaTime * cameraMoveSpeed) * m_CameraLocation;
			}
			if (GetInputManager()->IsKeyPressed(Key::E))
			{
				m_CameraLocation = glm::translate(FVector3(0.0f, 1.0f, 0.0f) * deltaTime * cameraMoveSpeed) * m_CameraLocation;
			}
		}

		FMatrix4 transform(1.0f);
		transform *= glm::translate(FVector3(m_CameraLocation));
		transform *= glm::rotate(cameraYaw, FVector3(0.0f, -1.0f, 0.0f));
		transform *= glm::rotate(cameraPitch, FVector3(-1.0f, 0.0f, 0.0f));
		m_Camera->SetTransform(transform);

		// Model transform
		FMatrix4 modelMatrix(1.0f);
		modelMatrix *= glm::translate(m_MeshLocation);
		modelMatrix *= glm::toMat4(FQuaternion(glm::radians(m_MeshRotation)));
		modelMatrix *= glm::scale(m_MeshScale);

		m_MeshCollada->SetTransform(modelMatrix);

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
					m_TextureCollada = Texture::Create(textureImage);
					m_TextureCollada->Bind(1);
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
			ImGui::DragFloat3("Rotation", &m_MeshRotation.x, 1.0f, -FLT_MAX, FLT_MAX);
			ImGui::DragFloat3("Scale", &m_MeshScale.x, 0.01f, -FLT_MAX, FLT_MAX);

			ImGui::DragFloat3("Light Direction Vector", &m_LightDirection.x, 0.01f, -1.0f, 1.0f);
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
					float yawDelta = event.GetX() * 0.2f;
					float pitchDelta = event.GetY() * 0.2f;

					m_CameraRotation.x = glm::clamp(m_CameraRotation.x + pitchDelta, -89.99f, 89.99f);
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
					ImGui::SetWindowFocus();

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
	TShared<Mesh> m_MeshCollada;
	TShared<Camera> m_Camera;
	TShared<Camera> m_AuxCamera;
	TShared<Texture> m_TextureCollada;
	TShared<Scene> m_Scene;
	FVector4 m_CameraLocation = { 0.0f, 0.0f, 2.0f, 1.0f };
	FVector3 m_CameraRotation = { 0.0f, 0.0f, 0.0f };
	FVector3 m_MeshLocation = { 0.0f, 0.0f, 0.0f };
	FVector3 m_MeshRotation = { -90.0f, 0.0f, 0.0f };
	FVector3 m_MeshScale = FVector3(1.0f);
	FVector3 m_AuxCameraLocation = { 0.0f, 0.0f, 4.0f };
	FVector3 m_LightDirection = glm::normalize(FVector3 { -0.2f, -0.4f, -0.8f });

	char m_TextureFileNameBuffer[MAX_PATH + 1];
};

USE_APPLICATION_CLASS(IonExample);

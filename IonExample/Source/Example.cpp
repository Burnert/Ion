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
		// @TODO: Move shader code to engine

		const char* vertSrc = R"(
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord0;

uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_InverseTranspose;
uniform vec3 u_CameraLocation;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_WorldNormal;
out vec3 v_PixelLocationWS;

void main()
{
	gl_Position = u_MVP * vec4(a_Position, 1.0);
	v_TexCoord = a_TexCoord0;
	v_Normal = a_Normal;
	v_WorldNormal = normalize(vec3(u_InverseTranspose * vec4(v_Normal, 0.0)));
	v_PixelLocationWS = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
}

)";

		// @TODO: Figure out how the hell to calculate light blending and all that

		const char* fragSrc = R"(
#version 430 core

struct DirectionalLight
{
	vec3 Direction;
	vec3 Color;
	float Intensity;
};

struct Light
{
	vec3 Location;
	vec3 Color;
	float Intensity;
	float Falloff;
};

#define MAX_LIGHTS 100

uniform sampler2D u_TextureSampler;
uniform vec3 u_CameraLocation;
uniform vec4 u_AmbientLightColor;
uniform DirectionalLight u_DirectionalLight;
uniform uint u_LightNum;
uniform Light u_Lights[MAX_LIGHTS];

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_WorldNormal;
in vec3 v_PixelLocationWS;

out vec4 Color;

void main()
{
	vec3 dirLightColor = max(dot(v_WorldNormal, -u_DirectionalLight.Direction), 0.0) * u_DirectionalLight.Color * u_DirectionalLight.Intensity;
	vec3 ambientLightColor = u_AmbientLightColor.xyz * u_AmbientLightColor.w;

	vec3 lightsColor = vec3(0.0);
	for (uint n = 0; n < u_LightNum; ++n)
	{
		Light light = u_Lights[n];

		vec3 inverseLightDir = normalize(light.Location - v_PixelLocationWS);
		float lightDistance = distance(light.Location, v_PixelLocationWS);
		float falloff = max((light.Falloff - lightDistance) / light.Falloff, 0.0);
		float lightIntensity = max(dot(inverseLightDir, v_WorldNormal), 0.0) * light.Intensity * falloff;
		
		vec3 lightColor = light.Color * lightIntensity;
		lightsColor += lightColor;
	}

	// Adding lights together seems like a really weird thing to do here
	vec3 finalLightColor = ambientLightColor + dirLightColor + lightsColor;

	Color = texture(u_TextureSampler, v_TexCoord).rgba * vec4(finalLightColor, 1.0);

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
		m_Camera->SetTransform(Math::Translate(FVector3(0.0f, 0.0f, 2.0f)));
		m_Camera->SetFOV(Math::Radians(90.0f));
		m_Camera->SetNearClip(0.1f);
		m_Camera->SetFarClip(100.0f);

		m_Scene = Scene::Create();
		m_Scene->SetActiveCamera(m_Camera);

		m_AuxCamera = Camera::Create();
		m_AuxCamera->SetTransform(Math::Translate(FVector3(0.0f, 0.0f, 4.0f)));
		m_AuxCamera->SetFOV(Math::Radians(66.0f));
		m_AuxCamera->SetNearClip(0.1f);
		m_AuxCamera->SetFarClip(10.0f);

		m_DirectionalLight = MakeShared<DirectionalLight>();
		m_Scene->SetActiveDirectionalLight(m_DirectionalLight.get());

		m_Light0->m_Location = Vector3(2.0f, 3.0f, -1.0f);
		m_Light0->m_Color = Vector3(1.0f, 1.0f, 1.0f);
		m_Light0->m_Intensity = 4.0f;
		m_Light0->m_Falloff = 5.0f;

		m_Light1->m_Location = Vector3(-1.0f, 1.0f, 2.0f);
		m_Light1->m_Color = Vector3(0.3f, 0.6f, 1.0f);
		m_Light1->m_Intensity = 3.0f;
		m_Light1->m_Falloff = 4.0f;

		m_Light2->m_Location = Vector3(2.0f, 1.0f, 4.0f);
		m_Light2->m_Color = Vector3(1.0f, 0.1f, 0.0f);
		m_Light2->m_Intensity = 2.0f;
		m_Light2->m_Falloff = 7.0f;

		m_Scene->AddLight(m_Light0.get());
		m_Scene->AddLight(m_Light1.get());
		m_Scene->AddLight(m_Light2.get());

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
		m_MeshCollada->SetTransform(Math::Rotate(Math::Radians(-90.0f), FVector3(1.0f, 0.0f, 0.0f)));

		m_Scene->AddDrawableObject(m_MeshCollada.get());

		File* dirTest = File::Create();
		dirTest->SetFilename(L"Assets");
		ionassert(dirTest->IsDirectory());
		std::vector<FileInfo> files = dirTest->GetFilesInDirectory();
		for (FileInfo& info : files)
		{
			LOG_INFO(L"{0}, {1}, {2}, {3}", info.Filename, info.FullPath, info.Size, info.bDirectory ? L"Dir" : L"File");
		}
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

		//m_LightDirection = Math::Normalize(m_LightDirection);
		//meshShader->SetUniform3f("u_LightDirection", m_LightDirection);

		m_Scene->SetAmbientLightColor(m_AmbientLightColor);

		m_DirectionalLightRotation = Quaternion(Math::Radians(m_DirectionalLightAngles));
		m_DirectionalLight->m_LightDirection = Math::Rotate(m_DirectionalLightRotation, Vector3(0.0f, 0.0f, -1.0f));
		m_DirectionalLight->m_Color = m_DirectionalLightColor;
		m_DirectionalLight->m_Intensity = m_DirectionalLightIntensity;

		float cameraMoveSpeed = 5.0f;

		if (GetInputManager()->IsMouseButtonPressed(Mouse::Right))
		{
			if (GetInputManager()->IsKeyPressed(Key::W))
			{
				m_CameraTransform += m_CameraTransform.GetForwardVector() * deltaTime * cameraMoveSpeed;
			}
			if (GetInputManager()->IsKeyPressed(Key::S))
			{
				m_CameraTransform += -m_CameraTransform.GetForwardVector() * deltaTime * cameraMoveSpeed;
			}
			if (GetInputManager()->IsKeyPressed(Key::A))
			{
				m_CameraTransform += -m_CameraTransform.GetRightVector() * deltaTime * cameraMoveSpeed;
			}
			if (GetInputManager()->IsKeyPressed(Key::D))
			{
				m_CameraTransform += m_CameraTransform.GetRightVector() * deltaTime * cameraMoveSpeed;
			}
			if (GetInputManager()->IsKeyPressed(Key::Q))
			{
				m_CameraTransform += Vector3(0.0f, -1.0f, 0.0f) * deltaTime * cameraMoveSpeed;
			}
			if (GetInputManager()->IsKeyPressed(Key::E))
			{
				m_CameraTransform += Vector3(0.0f, 1.0f, 0.0f) * deltaTime * cameraMoveSpeed;
			}
		}

		m_Camera->SetTransform(m_CameraTransform.GetMatrix());

		m_MeshTransform = { m_MeshLocation, m_MeshRotation, m_MeshScale };
		m_MeshCollada->SetTransform(m_MeshTransform.GetMatrix());

		c_Angle += deltaTime;
		c_Tint.y = (((c_Tint.y + deltaTime) >= 2.0f) ? 0.0f : (c_Tint.y + deltaTime));


		m_AuxCamera->SetTransform(Math::Translate(m_AuxCameraLocation));

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
		}
		ImGui::End();

		ImGui::Begin("Scene settings");
		{
			ImGui::DragFloat3("Directional Light Rotation", &m_DirectionalLightAngles.x, 1.0f, -180.0f, 180.0f);
			ImGui::DragFloat3("Directional Light Color", &m_DirectionalLightColor.x, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Directional Light Intensity", &m_DirectionalLightIntensity, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat4("Ambient Light Color", &m_AmbientLightColor.x, 0.01f, 0.0f, 1.0f);

			ImGui::DragFloat3("Light0 Location", &m_Light0->m_Location.x, 0.01f, -FLT_MAX, FLT_MAX);
			ImGui::DragFloat3("Light1 Location", &m_Light1->m_Location.x, 0.01f, -FLT_MAX, FLT_MAX);
			ImGui::DragFloat3("Light2 Location", &m_Light2->m_Location.x, 0.01f, -FLT_MAX, FLT_MAX);
		}
		ImGui::End();

		ImGui::Begin("Renderer Settings");
		{
			static const char* currentDrawMode = m_DrawModes[0];
			if (ImGui::BeginCombo("Draw Mode", currentDrawMode))
			{
				for (int i = 0; i < 3; ++i)
				{
					bool selected = currentDrawMode == m_DrawModes[i];
					if (ImGui::Selectable(m_DrawModes[i], selected))
					{
						currentDrawMode = m_DrawModes[i];
						if (currentDrawMode == m_DrawModes[0])
						{
							GetRenderer()->SetPolygonDrawMode(EPolygonDrawMode::Fill);
						}
						else if (currentDrawMode == m_DrawModes[1])
						{
							GetRenderer()->SetPolygonDrawMode(EPolygonDrawMode::Lines);
						}
						else if (currentDrawMode == m_DrawModes[2])
						{
							GetRenderer()->SetPolygonDrawMode(EPolygonDrawMode::Points);
						}
					}
					if (selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
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

					Rotator cameraRotation = m_CameraTransform.GetRotation();
					cameraRotation.SetPitch(Math::Clamp(cameraRotation.Pitch() - pitchDelta, -89.99f, 89.99f));
					cameraRotation.SetYaw(cameraRotation.Yaw() - yawDelta);
					m_CameraTransform.SetRotation(cameraRotation);

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
	TShared<DirectionalLight> m_DirectionalLight;
	TShared<Scene> m_Scene;

	FVector4 m_CameraLocation = { 0.0f, 0.0f, 2.0f, 1.0f };
	FVector3 m_CameraRotation = { 0.0f, 0.0f, 0.0f };
	Transform m_CameraTransform = { m_CameraLocation, m_CameraRotation, Vector3(1.0f) };

	FVector3 m_MeshLocation = { 0.0f, 0.0f, 0.0f };
	FVector3 m_MeshRotation = { -90.0f, 0.0f, 0.0f };
	FVector3 m_MeshScale = FVector3(1.0f);
	Transform m_MeshTransform = { m_MeshLocation, m_MeshRotation, m_MeshScale };

	FVector3 m_AuxCameraLocation = { 0.0f, 0.0f, 4.0f };

	//FVector3 m_LightDirection = Math::Normalize(FVector3 { -0.2f, -0.4f, -0.8f });

	Vector4 m_AmbientLightColor = Vector4(0.1f, 0.11f, 0.14f, 1.0f);

	Vector3 m_DirectionalLightAngles = Vector3(-30.0f, 30.0f, 0.0f);
	Vector3 m_DirectionalLightColor = Vector3(1.0f, 1.0f, 1.0f);
	float m_DirectionalLightIntensity = 0.0f;
	Quaternion m_DirectionalLightRotation = Quaternion(Math::Radians(m_DirectionalLightAngles));

	TShared<Light> m_Light0 = MakeShared<Light>();
	TShared<Light> m_Light1 = MakeShared<Light>();
	TShared<Light> m_Light2 = MakeShared<Light>();

	const char* m_DrawModes[3] = { "Triangles", "Lines", "Points" };

	char m_TextureFileNameBuffer[MAX_PATH + 1];
};

USE_APPLICATION_CLASS(IonExample);

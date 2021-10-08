/* 
* PBR Resources:
* https://learnopengl.com/PBR/Theory
* https://blog.selfshadow.com/publications/s2013-shading-course/hoffman/s2013_pbs_physics_math_notes.pdf
* https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
* https://www.shadertoy.com/view/4sSfzK
* http://www.codinglabs.net/article_physically_based_rendering.aspx
* http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx
* http://blog.wolfire.com/2015/10/Physically-based-rendering
*/

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

	static void InitExampleModel(TShared<Mesh>& mesh, TShared<Material>& material, TShared<Texture>& texture, 
		const TShared<Shader>& shader, TShared<Scene>& scene, const wchar* meshPath, const wchar* texturePath,
		const Matrix4& transform)
	{
		TUnique<File> file = TUnique<File>(File::Create(meshPath));
		ColladaDocument model(file.get());
		const ColladaData& data = model.GetData();
		TShared<VertexBuffer> vb = VertexBuffer::Create(data.VertexAttributes, data.VertexAttributeCount);
		vb->SetLayout(data.Layout);
		TShared<IndexBuffer> ib = IndexBuffer::Create(data.Indices, (uint32)data.IndexCount);

		File* tfile = File::Create(texturePath);
		Image* image = new Image;
		ionassertnd(image->Load(tfile));
		delete tfile;
		texture = Texture::Create(image);

		material = Material::Create();
		material->SetShader(shader);
		material->CreateParameter("Texture", EMaterialParameterType::Texture2D);
		material->SetParameter("Texture", texture);

		mesh = Mesh::Create();
		mesh->SetVertexBuffer(vb);
		mesh->SetIndexBuffer(ib);
		mesh->SetMaterial(material);
		mesh->SetTransform(transform);

		scene->AddDrawableObject(mesh.get());
	}

	virtual void OnInit() override
	{
		// @TODO: Figure out how the hell to calculate light blending and all that

#pragma warning(disable:6001)

		String vertSrc;
		String fragSrc;

		File::LoadToString(GetEnginePath() + L"/Shaders/Basic.vert", vertSrc);
		File::LoadToString(GetEnginePath() + L"/Shaders/Basic.frag", fragSrc);

		bool bResult;

		TShared<Shader> shader = Shader::Create();
		shader->AddShaderSource(EShaderType::Vertex, vertSrc);
		shader->AddShaderSource(EShaderType::Pixel, fragSrc);

		bResult = shader->Compile();
		ionassert(bResult);

		m_Camera = Camera::Create();
		m_Camera->SetTransform(Math::Translate(Vector3(0.0f, 0.0f, 2.0f)));
		m_Camera->SetFOV(Math::Radians(90.0f));
		m_Camera->SetNearClip(0.1f);
		m_Camera->SetFarClip(100.0f);

		m_Scene = Scene::Create();
		m_Scene->SetActiveCamera(m_Camera);

		m_AuxCamera = Camera::Create();
		m_AuxCamera->SetTransform(Math::Translate(Vector3(0.0f, 0.0f, 4.0f)));
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

		TShared<IndexBuffer> colladaIndexBuffer = IndexBuffer::Create(colladaData.Indices, (uint32)colladaData.IndexCount);

		WString textureFileName = L"char.png";
		memset(m_TextureFileNameBuffer, 0, sizeof(m_TextureFileNameBuffer));
		StringConverter::WCharToChar(textureFileName.c_str(), m_TextureFileNameBuffer);

		File* textureFile = File::Create(textureFileName);
		Image* textureImage = new Image;
		ionassertnd(textureImage->Load(textureFile));
		delete textureFile;

		m_TextureCollada = Texture::Create(textureImage);

		m_Material = Material::Create();
		m_Material->SetShader(shader);
		m_Material->CreateParameter("Texture", EMaterialParameterType::Texture2D);
		m_Material->SetParameter("Texture", m_TextureCollada);
		m_Material->CreateParameter("Texture1", EMaterialParameterType::Texture2D);
		m_Material->SetParameter("Texture1", m_TextureCollada);

		m_Material->RemoveParameter("Texture1");

		m_MeshCollada = Mesh::Create();
		m_MeshCollada->SetVertexBuffer(colladaVertexBuffer);
		m_MeshCollada->SetIndexBuffer(colladaIndexBuffer);
		m_MeshCollada->SetMaterial(m_Material);
		m_MeshCollada->SetTransform(Math::Rotate(Math::Radians(-90.0f), Vector3(1.0f, 0.0f, 0.0f)));

		m_Scene->AddDrawableObject(m_MeshCollada.get());

		// 4Pak
		InitExampleModel(m_Mesh4Pak, m_Material4Pak, m_Texture4Pak, shader, m_Scene, L"Assets/models/4pak.dae", L"Assets/textures/4pak.png",
			Math::Translate(Vector3(2.0f, 1.0f, 0.0f)) * Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 0.0f, 0.0f)))));

		// Piwsko
		InitExampleModel(m_MeshPiwsko, m_MaterialPiwsko, m_TexturePiwsko, shader, m_Scene, L"Assets/models/piwsko.dae", L"Assets/textures/piwsko.png",
			Math::Translate(Vector3(-2.0f, 1.0f, 0.0f)) * Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 0.0f, 0.0f)))));

		// Oscypek
		InitExampleModel(m_MeshOscypek, m_MaterialOscypek, m_TextureOscypek, shader, m_Scene, L"Assets/models/oscypek.dae", L"Assets/textures/oscypek.png",
			Math::Translate(Vector3(-1.0f, 1.0f, 1.0f))* Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 0.0f, 0.0f)))));

		// Ciupaga
		InitExampleModel(m_MeshCiupaga, m_MaterialCiupaga, m_TextureCiupaga, shader, m_Scene, L"Assets/models/ciupaga.dae", L"Assets/textures/ciupaga.png",
			Math::Translate(Vector3(1.0f, 1.0f, 1.0f))* Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 90.0f, 0.0f)))));

		// Slovak
		InitExampleModel(m_MeshSlovak, m_MaterialSlovak, m_TextureSlovak, shader, m_Scene, L"Assets/models/slovak.dae", L"Assets/textures/slovak.png",
			Math::Translate(Vector3(1.0f, 0.0f, -2.0f))* Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 180.0f, 0.0f)))));

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
		TShared<Material> meshMaterial = m_MeshCollada->GetMaterial().lock();
		TShared<Shader> meshShader = meshMaterial->GetShader();

		static float c_Angle = 0.0f;
		static Vector4 c_Tint(1.0f, 0.0f, 1.0f, 1.0f);

		// Perspective projection
		WindowDimensions dimensions = GetWindow()->GetDimensions();
		float aspectRatio = dimensions.GetAspectRatio();
		m_Camera->SetAspectRatio(aspectRatio);
		m_AuxCamera->SetAspectRatio(aspectRatio);

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

		m_MeshRotation.y = std::fmodf(m_MeshRotation.y + deltaTime * 90, 360);
		m_MeshTransform = { m_MeshLocation, m_MeshRotation, m_MeshScale };
		m_MeshCollada->SetTransform(m_MeshTransform.GetMatrix());

		c_Angle += deltaTime;
		c_Tint.y = (((c_Tint.y + deltaTime) >= 2.0f) ? 0.0f : (c_Tint.y + deltaTime));


		m_AuxCamera->SetTransform(Math::Translate(m_AuxCameraLocation));

		// ImGui:

		if (m_bDrawImGui)
		{
			//ImGui::ShowDemoWindow();
			//ImGui::ShowAboutWindow();
			//ImGui::ShowFontSelector("Fonts");
			//ImGui::ShowStyleEditor();
			//ImGui::Begin("Guide");
			//ImGui::ShowUserGuide();
			//ImGui::End();

			ImGui::ShowMetricsWindow();

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
						m_MeshCollada->GetMaterial().lock()->SetParameter("Texture", m_TextureCollada);
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
					for (int32 i = 0; i < 3; ++i)
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
	}

	virtual void OnRender() override
	{
		GetRenderer()->Clear(Vector4(0.1f, 0.1f, 0.1f, 1.0f));
		GetRenderer()->RenderScene(m_Scene);
	}

	virtual void OnShutdown() override
	{
	}

	virtual void OnEvent(const Event& event) override
	{
		m_EventDispatcher.Dispatch(this, event);
	}

	void OnKeyPressedEvent(const KeyPressedEvent& event)
	{
		if (event.GetKeyCode() == Key::F4)
		{
			m_bDrawImGui = !m_bDrawImGui;
		}
	}

	void OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event)
	{
		if (GetInputManager()->IsMouseButtonPressed(Mouse::Right))
		{
			float yawDelta = event.GetX() * 0.2f;
			float pitchDelta = event.GetY() * 0.2f;

			Rotator cameraRotation = m_CameraTransform.GetRotation();
			cameraRotation.SetPitch(Math::Clamp(cameraRotation.Pitch() - pitchDelta, -89.99f, 89.99f));
			cameraRotation.SetYaw(cameraRotation.Yaw() - yawDelta);
			m_CameraTransform.SetRotation(cameraRotation);
		}
	}

	void OnRawInputMouseButtonPressedEvent(const RawInputMouseButtonPressedEvent& event)
	{
		if (event.GetMouseButton() == Mouse::Right)
		{
			ImGui::SetWindowFocus();

			GetWindow()->LockCursor();
			GetWindow()->ShowCursor(false);
		}
	}

	void OnRawInputMouseButtonReleasedEvent(const RawInputMouseButtonReleasedEvent& event)
	{
		if (event.GetMouseButton() == Mouse::Right)
		{
			GetWindow()->UnlockCursor();
			GetWindow()->ShowCursor(true);
		}
	}

	using ExampleEventFunctions = TEventFunctionPack <
		TMemberEventFunction<IonExample, KeyPressedEvent, &IonExample::OnKeyPressedEvent>,
		TMemberEventFunction<IonExample, RawInputMouseMovedEvent, &IonExample::OnRawInputMouseMovedEvent>,
		TMemberEventFunction<IonExample, RawInputMouseButtonPressedEvent, &IonExample::OnRawInputMouseButtonPressedEvent>,
		TMemberEventFunction<IonExample, RawInputMouseButtonReleasedEvent, &IonExample::OnRawInputMouseButtonReleasedEvent>
	>;

private:
	TShared<Mesh> m_MeshCollada;
	TShared<Camera> m_Camera;
	TShared<Camera> m_AuxCamera;
	TShared<Material> m_Material;
	TShared<Texture> m_TextureCollada;
	TShared<DirectionalLight> m_DirectionalLight;
	TShared<Scene> m_Scene;

	TShared<Mesh> m_Mesh4Pak;
	TShared<Material> m_Material4Pak;
	TShared<Texture> m_Texture4Pak;

	TShared<Mesh> m_MeshPiwsko;
	TShared<Material> m_MaterialPiwsko;
	TShared<Texture> m_TexturePiwsko;

	TShared<Mesh> m_MeshCiupaga;
	TShared<Material> m_MaterialCiupaga;
	TShared<Texture> m_TextureCiupaga;

	TShared<Mesh> m_MeshOscypek;
	TShared<Material> m_MaterialOscypek;
	TShared<Texture> m_TextureOscypek;

	TShared<Mesh> m_MeshSlovak;
	TShared<Material> m_MaterialSlovak;
	TShared<Texture> m_TextureSlovak;

	Vector4 m_CameraLocation = { 0.0f, 0.0f, 2.0f, 1.0f };
	Vector3 m_CameraRotation = { 0.0f, 0.0f, 0.0f };
	Transform m_CameraTransform = { m_CameraLocation, m_CameraRotation, Vector3(1.0f) };

	Vector3 m_MeshLocation = { 0.0f, 0.0f, 0.0f };
	Vector3 m_MeshRotation = { -90.0f, 0.0f, 0.0f };
	Vector3 m_MeshScale = Vector3(1.0f);
	Transform m_MeshTransform = { m_MeshLocation, m_MeshRotation, m_MeshScale };

	Vector3 m_AuxCameraLocation = { 0.0f, 0.0f, 4.0f };

	Vector4 m_AmbientLightColor = Vector4(0.1f, 0.11f, 0.14f, 1.0f);

	Vector3 m_DirectionalLightAngles = Vector3(-30.0f, 30.0f, 0.0f);
	Vector3 m_DirectionalLightColor = Vector3(1.0f, 1.0f, 1.0f);
	float m_DirectionalLightIntensity = 1.0f;
	Quaternion m_DirectionalLightRotation = Quaternion(Math::Radians(m_DirectionalLightAngles));

	TShared<Light> m_Light0 = MakeShared<Light>();
	TShared<Light> m_Light1 = MakeShared<Light>();
	TShared<Light> m_Light2 = MakeShared<Light>();

	const char* m_DrawModes[3] = { "Triangles", "Lines", "Points" };

	char m_TextureFileNameBuffer[MAX_PATH + 1];

	bool m_bDrawImGui = true;

	EventDispatcher<ExampleEventFunctions> m_EventDispatcher;
};

USE_APPLICATION_CLASS(IonExample);
